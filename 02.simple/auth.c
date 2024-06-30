#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdlib.h>
#include <stdio.h>

struct pam_response *reply;

int function_conversation(
  int num_msg,
  const struct pam_message **msg,
  struct pam_response **resp,
  void *appdata_ptr
) {
  *resp = reply;
  return PAM_SUCCESS;
}

const struct pam_conv conv = { function_conversation, NULL };

int pam_authenticate_user(const char *username, const char *password) {
    pam_handle_t *handle = NULL;
    const char *service_name = "check_user";
    int retval;

    retval = pam_start(service_name, username, &conv, &handle); /* Initializing PAM */    
    reply = (struct pam_response *)malloc(sizeof(struct pam_response));
    reply[0].resp = strdup(password);
    reply[0].resp_retcode = 0;

    if (retval != PAM_SUCCESS){
      fprintf(stderr, "Failure in pam initialization: %s\n", pam_strerror(handle, retval));
      return 1;
    }

    retval = pam_authenticate(handle, 0); /* Do authentication (user will be asked for username and password)*/
    if (retval != PAM_SUCCESS) {
      fprintf(stderr, "Failure in pam authentication: %s\n", pam_strerror(handle, retval));
      return 1;
    }

    retval = pam_acct_mgmt(handle, 0); /* Do account management (check the account can access the system) */
    if (retval != PAM_SUCCESS) {
      fprintf(stderr, "Failure in pam account management: %s\n", pam_strerror(handle, retval));
      return 1;
    }

  pam_end(handle, retval);
  return 0;
}
