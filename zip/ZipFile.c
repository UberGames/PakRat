/*
 * @(#)ZipFile.c	1.12 99/05/25
 *
 * Copyright 1998 by Sun Microsystems, Inc.,
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
 * Native method support for java.util.zip.ZipFile
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include "jlong.h"
#include "jvm.h"
#include "jni.h"
#include "jni_util.h"
#include "zip_util.h"

#include "java_util_zip_ZipFile.h"
#include "java_util_jar_JarFile.h"

#define DEFLATED 8
#define STORED 0

static jfieldID jzfileID;
static int OPEN_READ = java_util_zip_ZipFile_OPEN_READ;
static int OPEN_DELETE = java_util_zip_ZipFile_OPEN_DELETE;

JNIEXPORT void JNICALL
Java_java_util_zip_ZipFile_initIDs(JNIEnv *env, jclass cls)
{
    jzfileID = (*env)->GetFieldID(env, cls, "jzfile", "J");
    assert(jzfileID != 0);
}

static void
ThrowZipException(JNIEnv *env, const char *msg)
{
    JNU_ThrowByName(env, "java/util/zip/ZipException", msg);
}

JNIEXPORT jlong JNICALL
Java_java_util_zip_ZipFile_open(JNIEnv *env, jclass cls, jstring name, jint mode)
{
    const char *path = JNU_GetStringPlatformChars(env, name, 0);
    jlong result = 0;
    int flag = 0;
    
    if (mode & OPEN_READ) flag |= O_RDONLY;
    if (mode & OPEN_DELETE) flag |= JVM_O_DELETE;

    if (path != 0) {
	char *msg;
	jzfile *zip = ZIP_Open_Generic(path, &msg, flag);
	JNU_ReleaseStringPlatformChars(env, name, path);
	if (zip != 0) {
	    result = ptr_to_jlong(zip);
	} else if (msg != 0) {
	    ThrowZipException(env, msg);
	} else if (errno == ENOMEM) {
	    JNU_ThrowOutOfMemoryError(env, 0);
	} else {
	    ThrowZipException(env, "error in opening zip file");
	}
    }
    return result;
}

JNIEXPORT jint JNICALL
Java_java_util_zip_ZipFile_getTotal(JNIEnv *env, jclass cls, jlong zfile)
{
    jzfile *zip = jlong_to_ptr(zfile);

    return zip->total;
}

JNIEXPORT void JNICALL
Java_java_util_zip_ZipFile_close(JNIEnv *env, jclass cls, jlong zfile)
{
    ZIP_Close(jlong_to_ptr(zfile));
}

JNIEXPORT jlong JNICALL
Java_java_util_zip_ZipFile_getEntry(JNIEnv *env, jclass cls, jlong zfile,
				    jstring name)
{
#define MAXNAME 1024
    jzfile *zip = jlong_to_ptr(zfile);
    jsize slen = (*env)->GetStringLength(env, name);
    jsize ulen = (*env)->GetStringUTFLength(env, name);
    char buf[MAXNAME+1], *path;
    jzentry *ze;

    if (ulen > MAXNAME) {
	path = malloc(ulen + 1);
	if (path == 0) {
	    JNU_ThrowOutOfMemoryError(env, 0);
	    return 0;
	}
    } else {
	path = buf;
    }
    (*env)->GetStringUTFRegion(env, name, 0, slen, path);
    path[ulen] = '\0';
    ze = ZIP_GetEntry(zip, path);
    if (path != buf) {
	free(path);
    }
    return ptr_to_jlong(ze);
}

JNIEXPORT void JNICALL
Java_java_util_zip_ZipFile_freeEntry(JNIEnv *env, jclass cls, jlong zfile,
				    jlong zentry)
{
    jzfile *zip = jlong_to_ptr(zfile);
    jzentry *ze = jlong_to_ptr(zentry);
    ZIP_FreeEntry(zip, ze);
}

JNIEXPORT jlong JNICALL
Java_java_util_zip_ZipFile_getNextEntry(JNIEnv *env, jclass cls, jlong zfile,
					jint n)
{
    jzentry *ze = ZIP_GetNextEntry(jlong_to_ptr(zfile), n);

    return ptr_to_jlong(ze);
}

JNIEXPORT jint JNICALL
Java_java_util_zip_ZipFile_getMethod(JNIEnv *env, jclass cls, jlong zentry)
{
    jzentry *ze = jlong_to_ptr(zentry);

    return ze->csize != 0 ? DEFLATED : STORED;
}

JNIEXPORT jint JNICALL
Java_java_util_zip_ZipFile_getCSize(JNIEnv *env, jclass cls, jlong zentry)
{
    jzentry *ze = jlong_to_ptr(zentry);

    return ze->csize != 0 ? ze->csize : ze->size;
}

JNIEXPORT jint JNICALL
Java_java_util_zip_ZipFile_getSize(JNIEnv *env, jclass cls, jlong zentry)
{
    jzentry *ze = jlong_to_ptr(zentry);

    return ze->size;
}

JNIEXPORT jint JNICALL
Java_java_util_zip_ZipFile_read(JNIEnv *env, jclass cls, jlong zfile,
				jlong zentry, jint pos, jbyteArray bytes,
				jint off, jint len)
{
#define BUFSIZE 8192
    jzfile *zip = jlong_to_ptr(zfile);
    jbyte buf[BUFSIZE];
    char *msg;

    if (len > BUFSIZE) {
	len = BUFSIZE;
    }
    ZIP_Lock(zip);
    len = ZIP_Read(zip, jlong_to_ptr(zentry), pos, buf, len);
    msg = zip->msg;
    ZIP_Unlock(zip);
    if (len == -1) {
	if (msg != 0) {
	    ThrowZipException(env, msg);
	} else {
	    JNU_ThrowIOException(env, strerror(errno));
	}
    } else {
	(*env)->SetByteArrayRegion(env, bytes, off, len, buf);
    }
    return len;
}

/*
 * Returns an array of strings representing the names of all entries
 * that begin with "META-INF/" (case ignored). This native method is
 * used in JarFile as an optimization when looking up manifest and
 * signature file entries. Returns null if no entries were found.
 */
JNIEXPORT jobjectArray JNICALL
Java_java_util_jar_JarFile_getMetaInfEntryNames(JNIEnv *env, jobject obj)
{
    jlong zfile = (*env)->GetLongField(env, obj, jzfileID);
    jzfile *zip;
    int i, count;
    jobjectArray result = 0;

    assert(zfile != 0);
    zip = jlong_to_ptr(zfile);

    /* count the number of valid ZIP metanames */
    count = 0;
    if (zip->metanames != 0) {
	for (i = 0; i < zip->metacount; i++) {
	    if (zip->metanames[i] != 0) {
		count++;
	    }
	}
    }

    /* If some names were found then build array of java strings */
    if (count > 0) {
	jclass cls = (*env)->FindClass(env, "java/lang/String");
	result = (*env)->NewObjectArray(env, count, cls, 0);
	if (result != 0) {
	    for (i = 0; i < count; i++) {
		jstring str = (*env)->NewStringUTF(env, zip->metanames[i]);
		if (str == 0) {
		    break;
		}
		(*env)->SetObjectArrayElement(env, result, i, str);
		(*env)->DeleteLocalRef(env, str);
	    }
	}
    }
    return result;
}
