#define _GNU_SOURCE
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>

#include <argon2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <syslog.h>

#define HASH_DIR "/etc/security/pin_hashes"

static int verify_pin(const char *pin, const char *hash) {
    int result = argon2_verify(hash, pin, strlen(pin), Argon2_id);
    return result == ARGON2_OK;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags,
                                   int argc, const char **argv) {
    const char *user;
    int pam_ret = pam_get_user(pamh, &user, "PIN: ");
    if (pam_ret != PAM_SUCCESS || user == NULL) {
        pam_syslog(pamh, LOG_ERR, "Failed to get username.");
        return PAM_AUTH_ERR;
    }

    char hash_path[512];
    snprintf(hash_path, sizeof(hash_path), "%s/%s.hash", HASH_DIR, user);

    FILE *f = fopen(hash_path, "r");
    if (!f) {
        pam_syslog(pamh, LOG_ERR, "Failed to open hash file '%s': %s", hash_path, strerror(errno));
        return PAM_AUTH_ERR;
    }

    char stored_hash[1024] = {0};
    if (!fgets(stored_hash, sizeof(stored_hash), f)) {
        pam_syslog(pamh, LOG_ERR, "Failed to read hash for user %s", user);
        fclose(f);
        return PAM_AUTH_ERR;
    }
    fclose(f);

    // Remove trailing newline if present
    stored_hash[strcspn(stored_hash, "\n")] = 0;

    const struct pam_message *pmsg;
    struct pam_response *resp;
    struct pam_conv *conv;

    pam_ret = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    if (pam_ret != PAM_SUCCESS || conv == NULL) {
        pam_syslog(pamh, LOG_ERR, "No conversation function.");
        return PAM_AUTH_ERR;
    }

    struct pam_message msg = {
        .msg_style = PAM_PROMPT_ECHO_OFF,
        .msg = "PIN: "
    };

    const struct pam_message *msgp = &msg;
    int resp_ret = conv->conv(1, &msgp, &resp, conv->appdata_ptr);
    if (resp_ret != PAM_SUCCESS || resp == NULL || resp->resp == NULL) {
        pam_syslog(pamh, LOG_ERR, "Failed to read PIN input.");
        return PAM_AUTH_ERR;
    }

    const char *pin_input = resp->resp;

    if (verify_pin(pin_input, stored_hash)) {
        free(resp->resp);
        free(resp);
        return PAM_SUCCESS;
    } else {
        pam_syslog(pamh, LOG_NOTICE, "Incorrect PIN for user %s", user);
        free(resp->resp);
        free(resp);
        return PAM_AUTH_ERR;
    }
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags,
                               int argc, const char **argv) {
    return PAM_SUCCESS;
}

