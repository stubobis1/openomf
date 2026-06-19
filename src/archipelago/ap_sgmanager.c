#include "archipelago/ap_sgmanager.h"

#include "formats/error.h"
#include "game/utils/settings.h"
#include "resources/resource_files.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <errno.h>
#include <string.h>

path get_ap_save_directory(void) {
    path name = get_save_directory();
    path_append(&name, "ap");
    if(!path_exists(&name)) {
        path_mkdir(&name);
    }
    return name;
}

int sg_save_as(sd_chr_file *chr, const char *filename_stem) {
    path save = get_save_directory();
    path_append(&save, filename_stem);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);
    const int ret = sd_chr_save(chr, &save);
    if(ret != SD_SUCCESS) {
        log_error("Saving pilot to %s failed: %s", path_c(&save), strerror(errno));
    } else {
        log_info("Saved pilot %s to %s", chr->pilot.name, path_c(&save));
        str stem;
        path_stem(&save, &stem);
        omf_free(settings_get()->tournament.last_name);
        settings_get()->tournament.last_name = omf_strdup(str_c(&stem));
        str_free(&stem);
        settings_save();
    }
    return ret;
}

int sg_save_ap(sd_chr_file *chr, const char *filename_stem) {
    path save = get_ap_save_directory();
    path_append(&save, filename_stem);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);
    const int ret = sd_chr_save(chr, &save);
    if(ret != SD_SUCCESS) {
        log_error("Saving AP pilot to %s failed: %s", path_c(&save), strerror(errno));
    } else {
        log_info("Saved AP pilot %s to %s", chr->pilot.name, path_c(&save));
        str stem;
        path_stem(&save, &stem);
        omf_free(settings_get()->tournament.last_name);
        settings_get()->tournament.last_name = omf_strdup(str_c(&stem));
        str_free(&stem);
        settings_save();
    }
    return ret;
}

int sg_load_ap_pilot(sd_chr_file *chr, const char *pilot_name) {
    path save = get_ap_save_directory();
    path_append(&save, pilot_name);
    path_set_ext(&save, ".CHR");
    path_dossify_filename(&save);
    if(!path_is_file(&save)) {
        return SD_FILE_OPEN_ERROR;
    }
    return sg_load(chr, &save);
}
