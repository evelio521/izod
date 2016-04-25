/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   marcos.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 11:07:58
 * @brief   
 *
 **/
#ifndef SERVER_BASE_MARCOS_H_
#define SERVER_BASE_MARCOS_H_

#include "compat.h"

/* Stops putting the code inside the SERVER namespace */
#define _END_SERVER_NAMESPACE_ }

/* Puts following code inside the SERVER namespace */
#define _START_SERVER_NAMESPACE_ namespace server {

/* Stops putting the code inside the CSTR namespace */
#define _END_CSTR_NAMESPACE_ }

/* Puts following code inside the CSTR namespace */
#define _START_CSTR_NAMESPACE_ namespace cstr {

/* Stops putting the code inside the BASE namespace */
#define _END_BASE_NAMESPACE_ }

/* Puts following code inside the BASE namespace */
#define _START_BASE_NAMESPACE_ namespace base {

/* use in util.cc*/
#define MEMBERSOF(x) (sizeof(x)/sizeof(x[0]))

// for url params
typedef map<string, string> In;

#endif  //  SERVER_BASE_MARCOS_H_
