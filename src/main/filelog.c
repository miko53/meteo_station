
#include "filelog.h"
#include "log.h"
#include "drivers/pcf_8523.h"
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "os.h"
#include "config.h"
#include <assert.h>

#define FILELOG_DEFAULT_PERMISSION      (0775)
#define FILELOG_SIZE_MAX                (256)
#define FILELOG_NB_MAX_MSG              (5)

static QueueHandle_t filelog_msgHandle = NULL;
static TaskHandle_t filelog_taskHandle = NULL;
static QueueHandle_t filelog_msgAllocatedHandle = NULL;
static bool filelog_activate;

static void filelog_task(void* arg);
static FILE* filelog_prepare_and_create_file(struct tm* pDate);
static bool filelog_date_has_changed(struct tm* pDate1, struct tm* pDate2);
static void filelog_build_filename(struct tm* pDate, char* filename, uint32_t size);
static FILE* filelog_create_file(char* filename);
static STATUS filelog_create_cascaded_folder(char* filename);
static void filelog_remove_filename(char* abspath);
static STATUS filelog_create_folder(char* path);
static STATUS filelog_pre_allocate_msg(QueueHandle_t* msgReserveQueue);
static void filelog_close_file(FILE** ppFile);
static void filelog_free_msg(filelog_msg* msg);

STATUS filelog_init(void)
{
  STATUS s;
  s = STATUS_OK;

  filelog_msgHandle = xQueueCreate(FILELOG_NB_MAX_MSG, sizeof(filelog_msg*));
  if (filelog_msgHandle == NULL)
    s = STATUS_ERROR;

  filelog_msgAllocatedHandle = xQueueCreate(FILELOG_NB_MAX_MSG, sizeof(filelog_msg*));
  if (filelog_msgAllocatedHandle == NULL)
    s = STATUS_ERROR;
  else
  {
    s = filelog_pre_allocate_msg(&filelog_msgAllocatedHandle);
  }

  if (s == STATUS_OK)
  {
    xTaskCreate(filelog_task, "filelog_task", FILELOG_THREAD_STACK_SIZE, NULL, FILELOG_THREAD_PRIORITY,
                &filelog_taskHandle);
    if (filelog_taskHandle == NULL)
      s = STATUS_ERROR;
  }

  return s;
}

STATUS filelog_pre_allocate_msg(QueueHandle_t* msgReserveQueue)
{
  STATUS s;
  BaseType_t rc;
  filelog_msg* pMsg;
  s = STATUS_OK;

  for (uint32_t i = 0; ((i < FILELOG_NB_MAX_MSG) && (s == STATUS_OK)); i++)
  {
    pMsg = calloc(1, sizeof(filelog_msg));
    if (pMsg != NULL)
    {
      rc = xQueueSend(*msgReserveQueue, &pMsg, 0);
      assert(rc == pdTRUE);
    }
    else
    {
      log_info_print("Unable to allocate msg (%d)", i);
      s = STATUS_ERROR;
      break;
    }
  }

  return s;
}

filelog_msg* filelog_allocate_msg()
{
  BaseType_t rc;
  filelog_msg* pMsg = NULL;
  rc = xQueueReceive(filelog_msgAllocatedHandle, &pMsg, 0);
  if (rc != pdTRUE)
    pMsg = NULL;
  return pMsg;
}

void filelog_free_msg(filelog_msg* msg)
{
  BaseType_t rc;
  rc = xQueueSend(filelog_msgAllocatedHandle, &msg, 0);
  assert(rc == pdTRUE);
}

STATUS filelog_write(filelog_msg* pData)
{
  STATUS s;
  BaseType_t rc;
  s = STATUS_ERROR;
  rc = xQueueSend(filelog_msgHandle, &pData, 0);
  if (rc == pdTRUE)
    s = STATUS_OK;

  return s;
}

static void filelog_task(void* arg)
{
  filelog_msg* msg;
  TickType_t delay;
  FILE* pCurrentFile = NULL;
  struct tm newDate;
  struct tm currentDate;
  bool isPreviouslyActivated;
  bool isActivated;

  delay = OS_WAIT_FOREVER;

  while (1)
  {
    if (xQueueReceive(filelog_msgHandle, &msg, delay) == pdTRUE)
    {
      isActivated = filelog_get_config();
      if ((isActivated == true) && (isPreviouslyActivated == false))
      {
        filelog_close_file(&pCurrentFile);
        pcf8523_get_date(&currentDate);
        pCurrentFile = filelog_prepare_and_create_file(&currentDate);
        isPreviouslyActivated = true;
      }

      if ((isActivated == false) && (isPreviouslyActivated == true))
      {
        filelog_close_file(&pCurrentFile);
      }

      if (isActivated)
      {
        log_info_print("filelog msg reception '%s'", msg->data);
        pcf8523_get_date(&newDate);
        if (filelog_date_has_changed(&newDate, &currentDate))
        {
          filelog_close_file(&pCurrentFile);
          currentDate = newDate;
          pCurrentFile = filelog_prepare_and_create_file(&currentDate);
        }

        if (pCurrentFile != NULL)
        {
          fputs(msg->data, pCurrentFile);
          fflush( pCurrentFile);
          fsync(fileno(pCurrentFile));
        }
      }

      filelog_free_msg(msg);
    }
  }
}

static bool filelog_date_has_changed(struct tm* pDate1, struct tm* pDate2)
{
  bool r;
  r = true;
  if ((pDate1->tm_year == pDate2->tm_year) &&
      (pDate1->tm_mon == pDate2->tm_mon) &&
      (pDate1->tm_wday == pDate2->tm_wday))
    r = false;
  return r;
}

static FILE* filelog_prepare_and_create_file(struct tm* pDate)
{
  STATUS s;
  FILE* pCurrentFile = NULL;
  char filename[FILELOG_SIZE_MAX];

  filelog_build_filename(pDate, filename, FILELOG_SIZE_MAX);

  pCurrentFile = filelog_create_file(filename);
  if (pCurrentFile == NULL)
  {
    s = filelog_create_cascaded_folder(filename);
    if (s == STATUS_OK)
      pCurrentFile = filelog_create_file(filename);
  }

  return pCurrentFile;
}


static void filelog_build_filename(struct tm* pDate, char* filename, uint32_t size)
{
  snprintf(filename, size, "%s/%d/%.2d/%.2d.txt", SD_CARD_MOUNT_POINT, pDate->tm_year + 1900, pDate->tm_mon + 1,
           pDate->tm_mday);
  //log_dbg_print("filename = %s", filename);
}


static FILE* filelog_create_file(char* filename)
{
  FILE* f = fopen(filename, "a");
  if (f == NULL)
  {
    //log_warning_print("unable to create/open file\n");
  }
  return f;
}

static void filelog_close_file(FILE** ppFile)
{
  if (*ppFile != NULL)
  {
    fclose(*ppFile);
    *ppFile = NULL;
  }
}

static STATUS filelog_create_cascaded_folder(char* filename)
{
  char foldername[FILELOG_SIZE_MAX];
  STATUS s;
  s = STATUS_OK;

  strncpy(foldername, filename, FILELOG_SIZE_MAX - 1);

  filelog_remove_filename(foldername);
  //log_dbg_print("foldername = %s\n", foldername);

  char* current_slash;

  //skip abs path + mount point
  if (foldername[0] == '/')
    current_slash = foldername + 1;

  current_slash = strchr(current_slash, '/');
  if (current_slash != NULL)
  {
    current_slash++;
    //log_dbg_print("current_slash = %s\n", current_slash);
  }
  else
    s = STATUS_ERROR; //abs path without another folder...

  if (s == STATUS_OK)
  {
    char cascadedFolder[FILELOG_SIZE_MAX];
    memcpy(cascadedFolder, foldername, current_slash - foldername);
    cascadedFolder[current_slash - foldername] = '\0';
    //log_dbg_print("dst1  = %s\n", cascadedFolder);

    while ((s == STATUS_OK) && (current_slash != NULL) && (*current_slash != '\0'))
    {
      current_slash = strchr(current_slash, '/');
      if (current_slash != NULL)
      {
        memcpy(cascadedFolder, foldername, current_slash - foldername);
        cascadedFolder[current_slash - foldername] = '\0';
        //log_dbg_print("dst  = %s\n", cascadedFolder);
        current_slash++;
        s = filelog_create_folder(cascadedFolder);
      }
      else
      {
        strncpy(cascadedFolder, foldername, FILELOG_SIZE_MAX - 1);
        //log_dbg_print("dst  = %s\n", cascadedFolder);
        s = filelog_create_folder(cascadedFolder);
      }
    }
  }

  return s;
}

static void filelog_remove_filename(char* abspath)
{
  char* slash;
  //remove filename
  slash = strrchr(abspath, '/');
  if (slash != NULL)
    *slash = '\0';
}

static STATUS filelog_create_folder(char* path)
{
  STATUS s;
  int rc;
  s = STATUS_OK;
  rc = mkdir(path, FILELOG_DEFAULT_PERMISSION);
  if (rc == -1)
  {
    if (errno != EEXIST)
    {
      s = STATUS_ERROR;
      log_info_print("unable to create folder %s err = %d\n", path, errno);
    }
  }
  return s;
}

void filelog_set_config(bool fileLogConfig)
{
  filelog_activate = fileLogConfig;
}

bool filelog_get_config(void)
{
  return filelog_activate;
}

