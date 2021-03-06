/*
 * Copyright 2013 10gen Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <string.h>

#include "mongoc-error.h"
#include "mongoc-sasl-private.h"


#ifndef SASL_CALLBACK_FN
#  define SASL_CALLBACK_FN(_f) ((int (*) (void))(_f))
#endif


void
_mongoc_sasl_set_mechanism (mongoc_sasl_t *sasl,
                            const char    *mechanism)
{
   BSON_ASSERT (sasl);

   free (sasl->mechanism);
   sasl->mechanism = mechanism ? strdup (mechanism) : NULL;
}


static int
_mongoc_sasl_get_pass (mongoc_sasl_t  *sasl,
                       int             param_id,
                       const char    **result,
                       unsigned       *result_len)
{
   BSON_ASSERT (sasl);
   BSON_ASSERT (param_id == SASL_CB_PASS);

   if (result) {
      *result = sasl->pass;
   }

   if (result_len) {
      *result_len = sasl->pass ? strlen (sasl->pass) : 0;
   }

   return (sasl->pass != NULL) ? SASL_OK : SASL_FAIL;
}


void
_mongoc_sasl_set_pass (mongoc_sasl_t *sasl,
                       const char    *pass)
{
   BSON_ASSERT (sasl);

   free (sasl->pass);
   sasl->pass = pass ? strdup (pass) : NULL;
}


static int
_mongoc_sasl_get_user (mongoc_sasl_t  *sasl,
                       int             param_id,
                       const char    **result,
                       unsigned       *result_len)
{
   BSON_ASSERT (sasl);
   BSON_ASSERT ((param_id == SASL_CB_USER) || (param_id == SASL_CB_AUTHNAME));

   if (result) {
      *result = sasl->user;
   }

   if (result_len) {
      *result_len = sasl->user ? strlen (sasl->user) : 0;
   }

   return (sasl->user != NULL) ? SASL_OK : SASL_FAIL;
}


void
_mongoc_sasl_set_user (mongoc_sasl_t *sasl,
                       const char    *user)
{
   BSON_ASSERT (sasl);

   free (sasl->user);
   sasl->user = user ? strdup (user) : NULL;
}


void
_mongoc_sasl_set_service_host (mongoc_sasl_t *sasl,
                               const char    *service_host)
{
   BSON_ASSERT (sasl);

   free (sasl->service_host);
   sasl->service_host = service_host ? strdup (service_host) : NULL;
}


void
_mongoc_sasl_set_service_name (mongoc_sasl_t *sasl,
                               const char    *service_name)
{
   BSON_ASSERT (sasl);

   free (sasl->service_name);
   sasl->service_name = service_name ? strdup (service_name) : NULL;
}


void
_mongoc_sasl_init (mongoc_sasl_t *sasl)
{
   sasl_callback_t callbacks [] = {
      { SASL_CB_AUTHNAME, SASL_CALLBACK_FN (_mongoc_sasl_get_user), sasl },
      { SASL_CB_USER, SASL_CALLBACK_FN (_mongoc_sasl_get_user), sasl },
      { SASL_CB_PASS, SASL_CALLBACK_FN (_mongoc_sasl_get_pass), sasl },
      { SASL_CB_LIST_END }
   };

   BSON_ASSERT (sasl);

   memset (sasl, 0, sizeof *sasl);

   memcpy (&sasl->callbacks, callbacks, sizeof callbacks);

   sasl->done = FALSE;
   sasl->step = 0;
   sasl->conn = NULL;
   sasl->mechanism = NULL;
   sasl->user = NULL;
   sasl->pass = NULL;
   sasl->service_name = NULL;
   sasl->service_host = NULL;
   sasl->interact = NULL;

   sasl_client_init (sasl->callbacks);
}


void
_mongoc_sasl_destroy (mongoc_sasl_t *sasl)
{
   BSON_ASSERT (sasl);

   if (sasl->conn) {
      sasl_dispose (&sasl->conn);
   }

   free (sasl->user);
   free (sasl->pass);
   free (sasl->mechanism);
   free (sasl->service_name);
   free (sasl->service_host);

   sasl_client_done ();
}


static bson_bool_t
_mongoc_sasl_is_failure (int           status,
                         bson_error_t *error)
{
   bson_bool_t ret = (status < 0);

   if (ret) {
      switch (status) {
      case SASL_NOMEM:
         bson_set_error (error,
                         MONGOC_ERROR_SASL,
                         status,
                         "SASL Failrue: insufficient memory.");
         break;
      case SASL_NOMECH:
         bson_set_error (error,
                         MONGOC_ERROR_SASL,
                         status,
                         "SASL Failure: failure to negotiate mechanism");
         break;
      case SASL_BADPARAM:
         bson_set_error (error,
                         MONGOC_ERROR_SASL,
                         status,
                         "Bad parameter supplied. Please file a bug "
                         "with mongo-c-driver.");
         break;
      default:
         bson_set_error (error,
                         MONGOC_ERROR_SASL,
                         status,
                         "SASL Failure: (%d): %s",
                         status,
                         sasl_errstring (status, NULL, NULL));
         break;
      }
   }

   return ret;
}


static bson_bool_t
_mongoc_sasl_start (mongoc_sasl_t      *sasl,
                    bson_uint8_t       *outbuf,
                    bson_uint32_t       outbufmax,
                    bson_uint32_t      *outbuflen,
                    bson_error_t       *error)
{
   const char *service_name = "mongodb";
   const char *service_host = "";
   const char *mechanism = NULL;
   const char *raw = NULL;
   unsigned raw_len = 0;
   int status;

   BSON_ASSERT (sasl);
   BSON_ASSERT (outbuf);
   BSON_ASSERT (outbufmax);
   BSON_ASSERT (outbuflen);

   if (sasl->service_name) {
      service_name = sasl->service_name;
   }

   if (sasl->service_host) {
      service_host = sasl->service_host;
   }

   status = sasl_client_new (service_name, service_host, NULL, NULL,
                             sasl->callbacks, 0, &sasl->conn);
   if (_mongoc_sasl_is_failure (status, error)) {
      return FALSE;
   }

   status = sasl_client_start (sasl->conn, sasl->mechanism,
                               &sasl->interact, &raw, &raw_len,
                               &mechanism);
   if (_mongoc_sasl_is_failure (status, error)) {
      return FALSE;
   }

   if ((0 != strcasecmp (mechanism, "GSSAPI")) &&
       (0 != strcasecmp (mechanism, "PLAIN"))) {
      bson_set_error (error,
                      MONGOC_ERROR_SASL,
                      SASL_NOMECH,
                      "SASL Failure: invalid mechanism \"%s\"",
                      mechanism);
      return FALSE;
   }


   status = sasl_encode64 (raw, raw_len, (char *)outbuf, outbufmax, outbuflen);
   if (_mongoc_sasl_is_failure (status, error)) {
      return FALSE;
   }

   return TRUE;
}


bson_bool_t
_mongoc_sasl_step (mongoc_sasl_t      *sasl,
                   const bson_uint8_t *inbuf,
                   bson_uint32_t       inbuflen,
                   bson_uint8_t       *outbuf,
                   bson_uint32_t       outbufmax,
                   bson_uint32_t      *outbuflen,
                   bson_error_t       *error)
{
   const char *raw = NULL;
   unsigned rawlen = 0;
   int status;

   BSON_ASSERT (sasl);
   BSON_ASSERT (inbuf);
   BSON_ASSERT (outbuf);
   BSON_ASSERT (outbuflen);
   BSON_ASSERT (*outbuflen);

   sasl->step++;

   if (sasl->step == 1) {
      return _mongoc_sasl_start (sasl, outbuf, outbufmax, outbuflen,
                                 error);
   } else if (sasl->step >= 10) {
      bson_set_error (error,
                      MONGOC_ERROR_SASL,
                      SASL_NOTDONE,
                      "SASL Failure: maximum steps detected");
      return FALSE;
   }

   if (!inbuflen) {
      bson_set_error (error,
                      MONGOC_ERROR_SASL,
                      MONGOC_ERROR_CLIENT_AUTHENTICATE,
                      "SASL Failure: no payload provided from server.");
      return FALSE;
   }

   status = sasl_decode64 ((char *)inbuf, inbuflen, (char *)outbuf, outbufmax,
                           outbuflen);
   if (_mongoc_sasl_is_failure (status, error)) {
      return FALSE;
   }

   status = sasl_client_step (sasl->conn, (char *)outbuf, *outbuflen,
                              &sasl->interact, &raw, &rawlen);
   if (_mongoc_sasl_is_failure (status, error)) {
      return FALSE;
   }

   status = sasl_encode64 (raw, rawlen, (char *)outbuf, outbufmax, outbuflen);
   if (_mongoc_sasl_is_failure (status, error)) {
      return FALSE;
   }

   return TRUE;
}
