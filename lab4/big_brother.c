#include "big_brother.h"
#include "fat_table.h"
#include "fat_types.h"
#include "fat_util.h"
#include "fat_volume.h"
#include <stdio.h>
#include <string.h>
#include "fat_file.h"

int
bb_is_log_file_dentry(fat_dir_entry dir_entry)
{
  return strncmp(LOG_FILE_BASENAME, (char*)(dir_entry->base_name), 3) == 0 &&
         strncmp(LOG_FILE_EXTENSION, (char*)(dir_entry->extension), 3) == 0;
}

int
bb_is_log_filepath(char* filepath)
{
  return strncmp(BB_LOG_FILE, filepath, 8) == 0;
}

int
bb_is_log_dirpath(char* filepath)
{
  return strncmp(BB_DIRNAME, filepath, 15) == 0;
}

/* Searches for a cluster that could correspond to the bb directory and returns
 * its index. If the cluster is not found, returns 0.
 */
u32
search_bb_orphan_dir_cluster(void)
{
  fat_volume vol = get_fat_volume();
  fat_table table = vol->table;
  u32 bb_dir_start_cluster = 2; /* First two clusters are reserved */
  u32 bytes_per_cluster = 0;
  off_t offset = 0;
  fat_dir_entry buf = NULL;

  bytes_per_cluster = fat_table_bytes_per_cluster(vol->table);
  buf = alloca(bytes_per_cluster);

  while (bb_dir_start_cluster < 789504) {
    if (le32_to_cpu(((const le32*)table->fat_map)[bb_dir_start_cluster]) == FAT_CLUSTER_BAD_SECTOR) {
      offset = fat_table_cluster_offset(vol->table, bb_dir_start_cluster);
      full_pread(table->fd, (void*)buf, sizeof(struct fat_dir_entry_s), offset);

      if (bb_is_log_file_dentry(buf))
        return bb_dir_start_cluster;
    }

    bb_dir_start_cluster++;
  }
  return 0;
}

int
bb_init_log_dir(u32 start_cluster)
{
  errno = 0;
  fat_volume vol = NULL;
  fat_tree_node root_node = NULL;

  vol = get_fat_volume();

  fat_file loaded_bb_dir = fat_file_init_orphan_dir(BB_DIRNAME, vol->table, start_cluster);

  fat_table_set_next_cluster(vol->table, start_cluster, FAT_CLUSTER_BAD_SECTOR);

  root_node = fat_tree_node_search(vol->file_tree, "/");
  vol->file_tree = fat_tree_insert(vol->file_tree, root_node, loaded_bb_dir);

  return -errno;
}
