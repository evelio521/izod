/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   status.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-05-26 15:26:10
 * @brief
 *
 **/
#ifndef UTIL_STATUS_H_
#define UTIL_STATUS_H_

#include <string>
#include "base/compat.h"
#include "base/marcos.h"

_START_UTIL_NAMESPACE_
_START_ERROR_NAMESPACE_
enum Code {
  OK = 0,
  CANCELLED = 1,
  UNKNOWN = 2,
  INVALID_ARGUMENT = 3,
  DEADLINE_EXCEEDED = 4,
  NOT_FOUND = 5,
  ALREADY_EXISTS = 6,
  PERMISSION_DENIED = 7,
  RESOURCE_EXHAUSTED = 8,
  FAILED_PRECONDITION = 9,
  ABORTED = 10,
  OUT_OF_RANGE = 11,
  UNIMPLEMENTED = 12,
  INTERNAL = 13,
  UNAVAILABLE = 14,
  DATA_LOSS = 15,
};

/*
 * Smallest code value (inclusive).
 */
const Code Code_MIN = OK;

/* 
 * Largest code value (inclusive).
 */
const Code Code_MAX = DATA_LOSS; 
_END_ERROR_NAMESPACE_

class Status {
 public:
  /*
   * Constructs a default OK status.
   */
  Status() : code_(util::error::OK) {}

  /*
   * Constructs a status with the given code and message.
   * @param[in] code The status code for the instance.
   * @param[in] msg If the code is other than OK then this should not be empty.
   */
  Status(util::error::Code code, const std::string& msg)
      : code_(code), msg_(msg) {}

  /*
   * Copy constructor.
   */
  Status(const Status& status) : code_(status.code_), msg_(status.msg_) {}

  /*
   * Standard destructor.
   */
  ~Status() {}

  /*
   * Assignment operator.
   */
  Status& operator =(const Status& status) {
    code_ = status.code_;
    msg_ = status.msg_;
    return *this;
  }

  /*
   * Equality operator.
   */
  bool operator ==(const Status& status) const {
    return code_ == status.code_ && msg_ == status.msg_;
  }

  /*
   * Determine if the status is ok().
   * @return true if the error code is OK, false otherwise.
   */
  bool ok() const { return code_ == util::error::OK; }

  /*
   * Get explanation bound at construction.
   */
  const std::string& error_message() const  { return msg_; }

  /*
   * Get error_code bound at construction.
   */
  util::error::Code error_code() const { return code_; }

  /*
   * Convert the status to a detailed string.
   *
   * If displaying the error to a user than error_message might be preferred
   * since it has less technical jargon.
   *
   * @see error_message()
   */
  std::string ToString() const;

  /*
   * This method is a NOP that confirms we are ignoring a status.
   */
  void IgnoreError() const {}

 private:
  util::error::Code code_;
  std::string msg_;
};

/*
 * Determine status error::Code to use from a standard Posix errno code.
 * @ingroup PlatformLayer
 *
 * This is more a suggestion than a definitive mapping.
 *
 * @param[in] errno_code A posix errno.
 * @return error code to use when creating a status for the Posix error.
 */
util::error::Code ErrnoCodeToStatusEnum(int errno_code);

/*
 * Create a status from a standard Posix errno code.
 * @ingroup PlatformLayer
 *
 * @param[in] errno_code A posix errno.
 * @param[in] msg A detailed message explanation can be empty to use a generic
 *            explanation based on the errno_code.
 * @return The status returned will be ok for errno_code 0, otherwise,
 *         it will be some form of failure.
 */
util::Status StatusFromErrno(int errno_code, const string& msg = "");

/*
 * Determine status error::Code to use from a standard HTTP response status
 * code.
 * @ingroup PlatformLayer
 *
 * This is more a suggestion than a definitive mapping.
 *
 * @param[in] http_code An HTTP response status code.
 * @return error code to use when creating a status for the HTTP status code.
 */
util::error::Code HttpCodeToStatusEnum(int http_code);

/*
 * Determine the standard HTTP error message for a given code.
 * @ingroup PlatformLayer
 *
 * @param[in] http_code An HTTP response status code.
 * @return short capitalized error phrase.
 */
const string HttpCodeToHttpErrorMessage(int http_code);

/*
 * Create a status from a standard HTTP response status code.
 * @ingroup PlatformLayer
 *
 * @param[in] http_code An HTTP status response code.
 * @param[in] msg A detailed message explanation can be empty to use a generic
 *            explanation based on the http_code.
 * @return The status returned will be ok for 2xx series responses, otherwise,
 *         it will be some form of failure.
 */
util::Status StatusFromHttp(int http_code, const string& msg = "");

/*
 * Shorthand notation for creating a status from a standard util::error enum
 * The symbol parameter is the symbolic enum name with the util::error
 * namespace stripped from it.
 */
#define STATUS_FROM_ENUM(symbol, msg) \
  util::Status(util::error::symbol, msg)

/*
 * Creates a standard OK status.
 */
inline util::Status  StatusOk() { return util::Status(); }

/*
 * Creates a standard ABORTED status.
 */
inline util::Status StatusAborted(const string& msg) {
  return STATUS_FROM_ENUM(ABORTED, msg);
}

/*
 * Creates a standard CANCELLED status.
 */
inline util::Status StatusCanceled(const string& msg) {
  return STATUS_FROM_ENUM(CANCELLED, msg);
}

/*
 * Creates a standard DATA_LOSS status.
 */
inline util::Status StatusDataLoss(const string& msg) {
  return STATUS_FROM_ENUM(DATA_LOSS, msg);
}

/*
 * Creates a standard DEADLINE_EXCEEDED status.
 */
inline util::Status StatusDeadlineExceeded(const string& msg) {
  return STATUS_FROM_ENUM(DEADLINE_EXCEEDED, msg);
}

/*
 * Creates a standard INTERNAL status.
 */
inline util::Status StatusInternalError(const string& msg) {
  return STATUS_FROM_ENUM(INTERNAL, msg);
}

/*
 * Creates a standard INVALID_ARGUMENT status.
 */
inline util::Status StatusInvalidArgument(const string& msg) {
  return STATUS_FROM_ENUM(INVALID_ARGUMENT, msg);
}

/*
 * Creates a standard OUT_OF_RANGE status.
 */
inline util::Status StatusOutOfRange(const string& msg) {
  return STATUS_FROM_ENUM(OUT_OF_RANGE, msg);
}

/*
 * Creates a standard PERMISSION_DENIED status.
 */
inline util::Status StatusPermissionDenied(const string& msg) {
  return STATUS_FROM_ENUM(PERMISSION_DENIED, msg);
}

/*
 * Creates a standard UNIMPLEMENTED status.
 */
inline util::Status StatusUnimplemented(const string& msg) {
  return STATUS_FROM_ENUM(UNIMPLEMENTED, msg);
}

/*
 * Creates a standard UNKNOWN status.
 */
inline util::Status StatusUnknown(const string& msg) {
  return STATUS_FROM_ENUM(UNKNOWN, msg);
}

/*
 * Creates a standard RESOURCE_EXHAUSTED status.
 */
inline util::Status StatusResourceExhausted(const string& msg) {
  return STATUS_FROM_ENUM(RESOURCE_EXHAUSTED, msg);
}

/*
 * Creates a standard FAILED_PRECONDITION status.
 */
inline util::Status StatusFailedPrecondition(const string& msg) {
  return STATUS_FROM_ENUM(FAILED_PRECONDITION, msg);
}

_END_UTIL_NAMESPACE_




































#endif  //__STATUS_H_

