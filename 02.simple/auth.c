#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdlib.h>

static struct pam_conv conv = {
  misc_conv, /* Conversation function defined in pam_misc.h */
  NULL /* We don't need additional data now*/
};

int pam_authenticate_user(const char *username) {
    pam_handle_t *handle = NULL;
    const char *service_name = "check_user";
    int retval;

    retval = pam_start(service_name, username, &conv, &handle); /* Initializing PAM */
    if (retval != PAM_SUCCESS){
      fprintf(stderr, "Failure in pam initialization: %s", pam_strerror(handle, retval));
      return 1;
    }

    retval = pam_authenticate(handle, 0); /* Do authentication (user will be asked for username and password)*/
    if (retval != PAM_SUCCESS) {
      fprintf(stderr, "Failure in pam authentication: %s", pam_strerror(handle, retval));
      return 1;
    }

    retval = pam_acct_mgmt(handle, 0); /* Do account management (check the account can access the system) */
    if (retval != PAM_SUCCESS) {
      fprintf(stderr, "Failure in pam account management: %s", pam_strerror(handle, retval));
      return 1;
    }

  pam_end(handle, retval);
  return 0;
}
