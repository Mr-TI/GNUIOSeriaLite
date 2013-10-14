/*
 *   Copyright 2013 Emeric Verschuur <emericv@openihs.org>
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *		   http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <jni.h>
#include <errno.h>
#include <string.h>
#include <gnu_io_serial_SerialDriver.h>

#define BREAK_ON_FAIL(exp) if ((exp) == -1) break
#define DEBUG(args...) if (debug) fprintf (stderr, args)
#define DEBUG_ARRAY(array, off, len) if (debug) _dumpByteArray(array, off, len)

int debug = 0;

void _dumpByteArray(void *array, int off, int len) {
	int i;
	for (i = off; i < len; i++) {
		fprintf(stderr, " %02X", ((char*)array)[i] & 0xFF);
	}
}

void _throwIOException(JNIEnv *env, const char *message) {
	jclass javaIoExcp = (*env)->FindClass(env, "java/io/IOException");
	if (javaIoExcp == NULL) {
		return;
	}
	(*env)->ThrowNew(env, javaIoExcp, message);
}

void _throwIllegalArgumentException(JNIEnv *env, const char *message) {
	jclass javaIoExcp = (*env)->FindClass(env, "java/lang/IllegalArgumentException");
	if (javaIoExcp == NULL) {
		printf("Class java.lang.IllegalArgumentException");
		return;
	}
	(*env)->ThrowNew(env, javaIoExcp, message);
}

#define _getStringRequiredArgOrBreak(ret, env, id, name) \
	if (id) ret = (*env)->GetStringUTFChars(env, id, 0);\
	if (ret == NULL) {\
		_throwIllegalArgumentException(env, "Invalid parameter "name": cannot be null");\
		break;\
	}\
	do { } while(0)

/*
 * Class:     gnu_io_SerialDriver
 * Method:    setDebugEnabled
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_gnu_io_serial_SerialDriver_setDebugEnabled
  (JNIEnv *env, jclass klass, jboolean enabled) {
	debug = enabled;
}
/*
 * Class:     gnu_io_SerialDriver
 * Method:    _open
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_gnu_io_serial_SerialDriver__1open
  (JNIEnv *env, jclass klass, jstring devNameId, jstring optionsId) {
	const char *devName = NULL;
	char *options = NULL;
	const char *error = NULL;
	char *key;
	char *val;
	char *next;
	char *parity = "none";
	char buff[32];
	int fd = -1;
	int ret = 0;
	int baudrate = 57600;
	int bitsperchar = 8;
	int stopbits = 1;
	int autocts = 1;
	int autorts = 1;
	int blocking = 1;
	struct termios config;
	do {
		_getStringRequiredArgOrBreak(devName, env, devNameId, "device name");
		DEBUG(" [ DD ] GNU IO Native serial driver: open device %s\n", devName);
		if (optionsId) {
			key = options = strdup((*env)->GetStringUTFChars(env, optionsId, 0));
			DEBUG(" [ DD ] GNU IO Native serial driver: options: %s\n", options);
			do {
				if (key[0] == '\0') {
					break;
				}
				val = strchr(key, '=');
				if (val[0] == '\0') {
					break;
				}
				val[0] = 0;
				val++;
				next = strchr(val, ';');
				if (next) {
					next[0] = 0;
					next++;
				}
				DEBUG(" [ DD ] GNU IO Native serial driver: option %s = %s\n", key, val);
				if (strcmp(key, "baudrate") == 0) {
					baudrate = atoi(val);
				} else if (strcmp(key, "bitsperchar") == 0) {
					bitsperchar = atoi(val);
				} else if (strcmp(key, "stopbits") == 0) {
					stopbits = atoi(val);
				} else if (strcmp(key, "parity") == 0) {
					parity = val;
				} else if (strcmp(key, "blocking") == 0) {
					blocking = (strcmp(val, "on") == 0? 1:0);
				} else if (strcmp(key, "autocts") == 0) {
					autocts = (strcmp(val, "on") == 0? 1:0);
				} else if (strcmp(key, "autorts") == 0) {
					autorts = (strcmp(val, "on") == 0? 1:0);
				} else {
					snprintf(buff, 31, "Invalid option %s", key);
					error = buff;
				}
			} while ((key = next) != NULL);
			free(options);
		}
		if (error != NULL) {
			break;
		}
		BREAK_ON_FAIL(fd = open(devName ,O_RDWR | O_NOCTTY | O_NDELAY));
		BREAK_ON_FAIL(fcntl(fd, F_SETFL, 0));
		BREAK_ON_FAIL(tcgetattr(fd, &config));
		cfmakeraw(&config);
		
		config.c_cflag |= (CLOCAL | CREAD);
		
		DEBUG(" [ DD ] GNU IO Native serial driver: set baudrate: %d\n", baudrate);
		switch (baudrate) {
			case 1200:
				ret =  cfsetispeed(&config,B1200);
				ret += cfsetospeed(&config,B1200);
				break;
			case 2400:
				ret =  cfsetispeed(&config,B2400);
				ret += cfsetospeed(&config,B2400);
				break;
			case 4800:
				ret =  cfsetispeed(&config,B4800);
				ret += cfsetospeed(&config,B4800);
				break;
			case 9600:
				ret =  cfsetispeed(&config,B9600);
				ret += cfsetospeed(&config,B9600);
				break;
			case 19200:
				ret =  cfsetispeed(&config,B19200);
				ret += cfsetospeed(&config,B19200);
				break;
			case 38400:
				ret =  cfsetispeed(&config,B38400);
				ret += cfsetospeed(&config,B38400);
				break;
			case 57600:
				ret =  cfsetispeed(&config,B57600);
				ret += cfsetospeed(&config,B57600);
				break;
			case 115200:
				ret =  cfsetispeed(&config,B115200);
				ret += cfsetospeed(&config,B115200);
				break;
			default:
				ret = -1;
				error = "Unsupported baud rate";
		}
		if (ret != 0) {
			break;
		}
		
		DEBUG(" [ DD ] GNU IO Native serial driver: set bitsperchar: %d\n", bitsperchar);
		switch (bitsperchar) {
			case 7:
				config.c_cflag |= CS7;
				break;
			case 8:
				config.c_cflag |= CS8;
				break;
			default:
				error = "Unsupported data size";
				ret = -1;
		}
		if (ret != 0) {
			break;
		}
		
		DEBUG(" [ DD ] GNU IO Native serial driver: set parity: %s\n", parity);
		switch (parity[0]) {
			case 'n':
			case 'N':
				config.c_cflag &= ~PARENB; // Clear parity enable
				break;
			case 'o':
			case 'O':
				config.c_cflag |= PARENB; // Parity enable
				config.c_cflag |= PARODD; // Enable odd parity 
				break;
			case 'e':
			case 'E':
				config.c_cflag |= PARENB; // Parity enable
				config.c_cflag &= ~PARODD; // Turn off odd parity = even
				break;
			default:
				error = "Unsupported parity\n";
				ret = -1;
		}
		if (ret != 0) {
			break;
		}
		
		DEBUG(" [ DD ] GNU IO Native serial driver: set stopbits: %d\n", stopbits);
		switch (stopbits) {
			case 1:
				config.c_cflag &= ~ CSTOPB;
				break;
			case 2:
				config.c_cflag |= CSTOPB;
				break;
			default:
				error = "Unsupported stop bits\n";
				ret = -1;
		}
		if (ret != 0) {
			break;
		}
		
		if (blocking == 0) {
			fprintf(stderr, " [ WW ] GNU IO Native serial driver: blocking=off not supported yet\n");
		}
		
		config.c_cc[VMIN] = 1;
		config.c_cc[VTIME] = 0;

		// Control flags
		if (autorts && autocts) {
			DEBUG(" [ DD ] GNU IO Native serial driver: RTS/CTS enabled\n");
			config.c_cflag |= CRTSCTS; // Enable RTS/CTS
		} else {
			DEBUG(" [ DD ] GNU IO Native serial driver: RTS/CTS disabled\n");
			config.c_cflag &= ~CRTSCTS; // Enable RTS/CTS
		}
		config.c_cflag |= CLOCAL;
		config.c_cflag |= CREAD;
		config.c_iflag &= ~(IXON | IXOFF | IXANY);
		
		BREAK_ON_FAIL(tcsetattr(fd, TCSANOW, &config));
		
		DEBUG(" [ DD ] GNU IO Native serial driver: open success\n");
		return fd;
	} while(0);
	if ((error != NULL && asprintf(&val, "Fail to open %s: %s", devName, error) != -1) 
		|| asprintf(&val, "Fail to open %s: %s", devName, strerror(errno)) != -1) {
		DEBUG(" [ DD ] GNU IO Native serial driver: open failure with error: %s\n", val);
		_throwIOException(env, val);
		free(val);
	}
	if (fd != -1) {
		close(fd);
	}
	return fd;
}

/*
 * Class:     gnu_io_SerialDriver
 * Method:    _close
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_gnu_io_serial_SerialDriver__1close
  (JNIEnv *env, jclass klass, jint fd) {
	DEBUG(" [ DD ] GNU IO Native serial driver: close\n");
	close(fd);
}

/*
 * Class:     gnu_io_SerialDriver
 * Method:    _available
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_gnu_io_serial_SerialDriver__1available
  (JNIEnv *env, jclass klass, jint fd) {
	char *error_message;
	int result;
	if( ioctl( fd, FIONREAD, &result ) < 0 ) {
		if (asprintf(&error_message, "Fail to write: %s", strerror(errno)) != -1) {
			DEBUG(" [ DD ] GNU IO Native serial driver: get available failure with error: %s\n", error_message);
			_throwIOException(env, error_message);
			free(error_message);
		}
	}
	DEBUG(" [ DD ] GNU IO Native serial driver: available: %d\n", result);
	return result;
}

/*
 * Class:     gnu_io_SerialDriver
 * Method:    _read
 * Signature: (I[BII)I
 */
JNIEXPORT jint JNICALL Java_gnu_io_serial_SerialDriver__1read__I_3BII
  (JNIEnv *env, jclass klass, jint fd, jbyteArray b, jint off, jint len) {
	char *error_message;
	int ret = 0,readCnt = 0;
	jbyte* data = (*env)->GetByteArrayElements( env, b, 0 );
	do {
		ret = read(fd, (void * ) ((char *) data + readCnt + off), len - readCnt);
		if(ret > 0){
			readCnt += ret;
		}
	}  while ((readCnt < len ) && (ret > 0 || errno == EINTR));
	if (ret < 0) {
		if (asprintf(&error_message, "Fail to write: %s", strerror(errno)) != -1) {
			DEBUG(" [ DD ] GNU IO Native serial driver: read byte array failure with error: %s\n", error_message);
			_throwIOException(env, error_message);
			free(error_message);
		}
	}
	DEBUG(" [ DD ] GNU IO Native serial driver: read byte array:");
	DEBUG_ARRAY(data, off, len);
	DEBUG("\n");
	(*env)->ReleaseByteArrayElements(env, b, data, 0);
	return readCnt;
}

/*
 * Class:     gnu_io_SerialDriver
 * Method:    _read
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_gnu_io_serial_SerialDriver__1read__I
  (JNIEnv *env, jclass klass, jint fd) {
	char *error_message;
	int ret;
	char data;
	ret = read(fd, &data, 1);
	if (ret < 0) {
		if (asprintf(&error_message, "Fail to write: %s", strerror(errno)) != -1) {
			DEBUG(" [ DD ] GNU IO Native serial driver: read byte failure with error: %s\n", error_message);
			_throwIOException(env, error_message);
			free(error_message);
		}
	}
	DEBUG(" [ DD ] GNU IO Native serial driver: read byte: %02X\n", (ret? data: -1) & 0xFF);
	return ret? (data & 0xFF): -1;
}

/*
 * Class:     gnu_io_SerialDriver
 * Method:    _write
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_gnu_io_serial_SerialDriver__1write__II
  (JNIEnv *env, jclass klass, jint fd, jint b) {
	char *error_message;
	char data = (char)b;
	if (write(fd, &data, 1) < 0) {
		if (asprintf(&error_message, "Fail to write: %s", strerror(errno)) != -1) {
			DEBUG(" [ DD ] GNU IO Native serial driver: write byte failure with error: %s\n", error_message);
			_throwIOException(env, error_message);
			free(error_message);
		}
	}
	DEBUG(" [ DD ] GNU IO Native serial driver: write byte: %02X\n", b & 0xFF);
}

/*
 * Class:     gnu_io_SerialDriver
 * Method:    _write
 * Signature: (I[BII)V
 */
JNIEXPORT void JNICALL Java_gnu_io_serial_SerialDriver__1write__I_3BII
  (JNIEnv *env, jclass klass, jint fd, jbyteArray b, jint off, jint len) {
	char *error_message;
	int ret = 0,writeCnt = 0;
	jbyte* data = (*env)->GetByteArrayElements(env, b, 0);
	do {
		ret = write(fd, (void * ) ((char *) data + writeCnt + off), len - writeCnt);
		if(ret > 0){
			writeCnt += ret;
		}
	}  while ((writeCnt < len ) && (ret > 0 || errno == EINTR));
	if (ret < 0) {
		if (asprintf(&error_message, "Fail to write: %s", strerror(errno)) != -1) {
			DEBUG(" [ DD ] GNU IO Native serial driver: write byte array failure with error: %s\n", error_message);
			_throwIOException(env, error_message);
			free(error_message);
		}
	}
	DEBUG(" [ DD ] GNU IO Native serial driver: write byte array:");
	DEBUG_ARRAY(data, off, len);
	DEBUG("\n");
	(*env)->ReleaseByteArrayElements(env, b, data, 0);
}
