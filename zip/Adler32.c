/*
 * @(#)Adler32.c	1.8 98/09/21
 *
 * Copyright 1997, 1998 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

/*
 * Native method support for java.util.zip.Adler32
 */

#include "jni.h"
#include "jni_util.h"
#include "zlib.h"

#include "java_util_zip_Adler32.h"

JNIEXPORT jint JNICALL
Java_java_util_zip_Adler32_update(JNIEnv *env, jclass cls, jint adler, jint b)
{
    Bytef buf[1];

    buf[0] = (Bytef)b;
    return adler32(adler, buf, 1);
}

JNIEXPORT jint JNICALL
Java_java_util_zip_Adler32_updateBytes(JNIEnv *env, jclass cls, jint adler,
				       jarray b, jint off, jint len)
{
    Bytef *buf = (*env)->GetPrimitiveArrayCritical(env, b, 0);
    if (buf) {
        adler = adler32(adler, buf + off, len);
	(*env)->ReleasePrimitiveArrayCritical(env, b, buf, 0);
    }
    return adler;
}

