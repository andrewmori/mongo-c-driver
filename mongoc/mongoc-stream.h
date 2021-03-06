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


#if !defined (MONGOC_INSIDE) && !defined (MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif


#ifndef MONGOC_STREAM_H
#define MONGOC_STREAM_H


#include <bson.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>


BSON_BEGIN_DECLS


typedef struct _mongoc_stream_t mongoc_stream_t;


int              mongoc_stream_close         (mongoc_stream_t *stream);
int              mongoc_stream_cork          (mongoc_stream_t *stream);
int              mongoc_stream_uncork        (mongoc_stream_t *stream);
void             mongoc_stream_destroy       (mongoc_stream_t *stream);
int              mongoc_stream_flush         (mongoc_stream_t *stream);
ssize_t          mongoc_stream_writev        (mongoc_stream_t *stream,
                                              struct iovec    *iov,
                                              size_t           iovcnt,
                                              bson_int32_t     timeout_msec);
ssize_t          mongoc_stream_readv         (mongoc_stream_t *stream,
                                              struct iovec    *iov,
                                              size_t           iovcnt,
                                              size_t           min_bytes,
                                              bson_int32_t     timeout_msec);
ssize_t          mongoc_stream_read          (mongoc_stream_t *stream,
                                              void            *buf,
                                              size_t           count,
                                              size_t           min_bytes,
                                              bson_int32_t     timeout_msec);
int              mongoc_stream_setsockopt    (mongoc_stream_t *stream,
                                              int              level,
                                              int              optname,
                                              void            *optval,
                                              socklen_t        optlen);


BSON_END_DECLS


#endif /* MONGOC_STREAM_H */
