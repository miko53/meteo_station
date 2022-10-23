
#include "filelog.h"
#include "log.h"
#include "drivers/pcf_8523.h"
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "config.h"

#define FILELOG_DEFAULT_PERMISSION      (0775)
#define FILELOG_SIZE_MAX                (256)

void filelog_build_filename(char* filename, uint32_t size);
STATUS filelog_create_file(char* filename);
STATUS filelog_create_cascaded_folder(char* filename);

STATUS filelog_init(void)
{
  STATUS s;
  s = STATUS_OK;
  char filename[FILELOG_SIZE_MAX];

  filelog_build_filename(filename, FILELOG_SIZE_MAX);

  s = filelog_create_file(filename);
  if (s != STATUS_OK)
  {
    s = filelog_create_cascaded_folder(filename);
    if (s == STATUS_OK)
      s = filelog_create_file(filename);
  }
  return s;
}

void filelog_build_filename(char* filename, uint32_t size)
{
  struct tm currentDate;
  pcf8523_get_date(&currentDate);
  snprintf(filename, size, "%s/%d/%d/%d.txt", SD_CARD_MOUNT_POINT, currentDate.tm_year + 1900, currentDate.tm_mon + 1,
           currentDate.tm_wday);
  log_dbg_print("filename = %s", filename);
}


STATUS filelog_create_file(char* filename)
{
  STATUS s;
  s = STATUS_OK;
  FILE* f = fopen(filename, "a");
  if (f == NULL)
  {
    log_dbg_print("unable to create/open file\n");
    s = STATUS_ERROR;
  }
  return s;
}

void filelog_remove_filename(char* abspath)
{
  char* slash;
  //remove filename
  slash = strrchr(abspath, '/');
  if (slash != NULL)
    *slash = '\0';
}

STATUS filelog_create_folder(char* path)
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
      log_dbg_print("unable to create folder %s err = %d\n", path, errno);
    }
  }
  return s;
}

STATUS filelog_create_cascaded_folder(char* filename)
{
  char foldername[FILELOG_SIZE_MAX];
  STATUS s;
  s = STATUS_OK;

  strncpy(foldername, filename, FILELOG_SIZE_MAX - 1);

  filelog_remove_filename(foldername);
  log_dbg_print("foldername = %s\n", foldername);

  char* current_slash;

  //skip abs path + mount point
  if (foldername[0] == '/')
  {
    current_slash = foldername + 1;
  }
  current_slash = strchr(current_slash, '/');
  if (current_slash != NULL)
  {
    current_slash++;
    log_dbg_print("current_slash = %s\n", current_slash);
  }
  else
    s = STATUS_ERROR; //abs path without another folder...

  if (s == STATUS_OK)
  {
    char cascadedFolder[FILELOG_SIZE_MAX];
    memcpy(cascadedFolder, foldername, current_slash - foldername);
    cascadedFolder[current_slash - foldername] = '\0';
    log_dbg_print("dst1  = %s\n", cascadedFolder);

    while ((s == STATUS_OK) && (current_slash != NULL) && (*current_slash != '\0'))
    {
      current_slash = strchr(current_slash, '/');
      if (current_slash != NULL)
      {
        memcpy(cascadedFolder, foldername, current_slash - foldername);
        cascadedFolder[current_slash - foldername] = '\0';
        log_dbg_print("dst  = %s\n", cascadedFolder);
        current_slash++;
        s = filelog_create_folder(cascadedFolder);
      }
      else
      {
        strncpy(cascadedFolder, foldername, FILELOG_SIZE_MAX - 1);
        log_dbg_print("dst  = %s\n", cascadedFolder);
        s = filelog_create_folder(cascadedFolder);
      }
    }
  }

  return s;
}
