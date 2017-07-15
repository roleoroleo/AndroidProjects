#include "AndroidGL20.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>

static int initialized = 0;

static jclass nioAccessClass;
static jclass bufferClass;
static jclass OOMEClass;
static jclass UOEClass;
static jclass IAEClass;

static jmethodID getBasePointerID;
static jmethodID getBaseArrayID;
static jmethodID getBaseArrayOffsetID;
static jmethodID allowIndirectBuffersID;
static jfieldID positionID;
static jfieldID limitID;
static jfieldID elementSizeShiftID;


static void
nativeClassInitBuffer(JNIEnv *_env)
{
    jclass nioAccessClassLocal = _env->FindClass("java/nio/NIOAccess");
    nioAccessClass = (jclass) _env->NewGlobalRef(nioAccessClassLocal);

    jclass bufferClassLocal = _env->FindClass("java/nio/Buffer");
    bufferClass = (jclass) _env->NewGlobalRef(bufferClassLocal);

    getBasePointerID = _env->GetStaticMethodID(nioAccessClass, "getBasePointer", "(Ljava/nio/Buffer;)J");
    getBaseArrayID = _env->GetStaticMethodID(nioAccessClass, "getBaseArray", "(Ljava/nio/Buffer;)Ljava/lang/Object;");
    getBaseArrayOffsetID = _env->GetStaticMethodID(nioAccessClass,"getBaseArrayOffset", "(Ljava/nio/Buffer;)I");
    positionID = _env->GetFieldID(bufferClass, "position", "I");
    limitID = _env->GetFieldID(bufferClass, "limit", "I");
    elementSizeShiftID = _env->GetFieldID(bufferClass, "_elementSizeShift", "I");
}

static void
nativeClassInit(JNIEnv *_env)
{
    nativeClassInitBuffer(_env);

    jclass IAEClassLocal =
        _env->FindClass("java/lang/IllegalArgumentException");
    jclass OOMEClassLocal =
         _env->FindClass("java/lang/OutOfMemoryError");
    jclass UOEClassLocal =
         _env->FindClass("java/lang/UnsupportedOperationException");

    IAEClass = (jclass) _env->NewGlobalRef(IAEClassLocal);
    OOMEClass = (jclass) _env->NewGlobalRef(OOMEClassLocal);
    UOEClass = (jclass) _env->NewGlobalRef(UOEClassLocal);
}

static void *
getDirectBufferPointer(JNIEnv *_env, jobject buffer) {
    if (!buffer) {
        return NULL;
    }
    void* buf = _env->GetDirectBufferAddress(buffer);
    if (buf) {
        jint position = _env->GetIntField(buffer, positionID);
        jint elementSizeShift = _env->GetIntField(buffer, elementSizeShiftID);
        buf = ((char*) buf) + (position << elementSizeShift);
    } else {
        _env->ThrowNew(IAEClass, "Must use a native order direct Buffer");
    }
    return buf;
}

static const char* getString( JNIEnv *env, jstring string )
{
	return (const char*)env->GetStringUTFChars(string, NULL);
}

static void releaseString( JNIEnv *env, jstring string, const char* cString )
{
	env->ReleaseStringUTFChars(string, cString);
}

static void *
getPointer(JNIEnv *_env, jobject buffer, jarray *array, jint *remaining)
{
    jint position;
    jint limit;
    jint elementSizeShift;
    jlong pointer;
    jint offset;
    void *data;

    position = _env->GetIntField(buffer, positionID);
    limit = _env->GetIntField(buffer, limitID);
    elementSizeShift = _env->GetIntField(buffer, elementSizeShiftID);
    *remaining = (limit - position) << elementSizeShift;
    pointer = _env->CallStaticLongMethod(nioAccessClass,
            getBasePointerID, buffer);
    if (pointer != 0L) {
        *array = NULL;
        return (void *) (jint) pointer;
    }

    *array = (jarray) _env->CallStaticObjectMethod(nioAccessClass,
            getBaseArrayID, buffer);
    if (*array == NULL) {
        return (void*) NULL;
    }
    offset = _env->CallStaticIntMethod(nioAccessClass,
            getBaseArrayOffsetID, buffer);
    data = _env->GetPrimitiveArrayCritical(*array, (jboolean *) 0);

    return (void *) ((char *) data + offset);
}

static void
releasePointer(JNIEnv *_env, jarray array, void *data, jboolean commit)
{
    _env->ReleasePrimitiveArrayCritical(array, data,
					   commit ? 0 : JNI_ABORT);
}

static int
getNumCompressedTextureFormats() {
    int numCompressedTextureFormats = 0;
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTextureFormats);
    return numCompressedTextureFormats;
}

JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_init
  (JNIEnv *env, jclass)
{
	nativeClassInit( env );
	__android_log_print(ANDROID_LOG_INFO, "GL2", "all initialized %d", 2);
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glActiveTexture
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glActiveTexture
  (JNIEnv *, jobject, jint texture)
{
	glActiveTexture( texture );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glAttachShader
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glAttachShader
  (JNIEnv *, jobject, jint program, jint shader)
{
	glAttachShader( program, shader );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBindAttribLocation
 * Signature: (IILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBindAttribLocation
  (JNIEnv *env, jobject, jint program, jint index, jstring name)
{
	const char* namePtr = getString( env, name );
	glBindAttribLocation( program, index, namePtr );
	releaseString( env, name, namePtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBindBuffer
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBindBuffer
  (JNIEnv *env, jobject, jint target, jint buffer)
{
	glBindBuffer( target, buffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBindFramebuffer
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBindFramebuffer
  (JNIEnv *env, jobject, jint target, jint framebuffer)
{
	glBindFramebuffer( target, framebuffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBindRenderbuffer
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBindRenderbuffer
  (JNIEnv *env, jobject, jint target, jint renderbuffer)
{
	glBindRenderbuffer( target, renderbuffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBindTexture
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBindTexture
  (JNIEnv *env, jobject, jint target, jint texture)
{
	glBindTexture( target, texture );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBlendColor
 * Signature: (FFFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBlendColor
  (JNIEnv *env, jobject, jfloat red, jfloat green, jfloat blue, jfloat alpha)
{
	glBlendColor( red, green, blue, alpha );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBlendEquation
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBlendEquation
  (JNIEnv *env, jobject, jint mode)
{
	glBlendEquation( mode );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBlendEquationSeparate
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBlendEquationSeparate
  (JNIEnv *env, jobject, jint modeRGB, jint modeAlpha)
{
	glBlendEquationSeparate( modeRGB, modeAlpha );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBlendFunc
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBlendFunc
  (JNIEnv *env, jobject, jint sfactor, jint dfactor)
{
	glBlendFunc( sfactor, dfactor );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBlendFuncSeparate
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBlendFuncSeparate
  (JNIEnv *env, jobject, jint srcRGB, jint dstRGB, jint srcAlpha, jint dstAlpha)
{
	glBlendFuncSeparate( srcRGB, dstRGB, srcAlpha, dstAlpha);
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBufferData
 * Signature: (IILjava/nio/Buffer;I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBufferData
(JNIEnv *_env, jobject _this, jint target, jint size, jobject data_buf, jint usage) {
    jarray _array = (jarray) 0;
    jint _remaining;
    GLvoid *data = (GLvoid *) 0;

    if (data_buf) {
        data = (GLvoid *)getPointer(_env, data_buf, &_array, &_remaining);
        if (_remaining < size) {
            _env->ThrowNew(IAEClass, "remaining() < size");
            goto exit;
        }
    }
    glBufferData(
        (GLenum)target,
        (GLsizeiptr)size,
        (GLvoid *)data,
        (GLenum)usage
    );

exit:
    if (_array) {
        releasePointer(_env, _array, data, JNI_FALSE);
    }
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glBufferSubData
 * Signature: (IIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glBufferSubData
(JNIEnv *_env, jobject _this, jint target, jint offset, jint size, jobject data_buf) {
  jarray _array = (jarray) 0;
  jint _remaining;
  GLvoid *data = (GLvoid *) 0;

  data = (GLvoid *)getPointer(_env, data_buf, &_array, &_remaining);
  if (_remaining < size) {
      _env->ThrowNew(IAEClass, "remaining() < size");
      goto exit;
  }
  glBufferSubData(
      (GLenum)target,
      (GLintptr)offset,
      (GLsizeiptr)size,
      (GLvoid *)data
  );

exit:
  if (_array) {
      releasePointer(_env, _array, data, JNI_FALSE);
  }
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCheckFramebufferStatus
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCheckFramebufferStatus
  (JNIEnv *env, jobject, jint target)
{
	return glCheckFramebufferStatus( target );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glClear
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glClear
  (JNIEnv *env, jobject, jint mask)
{
	glClear( mask );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glClearColor
 * Signature: (FFFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glClearColor
  (JNIEnv *env, jobject, jfloat red, jfloat green, jfloat blue, jfloat alpha)
{
	glClearColor( red, green, blue, alpha );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glClearDepthf
 * Signature: (F)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glClearDepthf
  (JNIEnv *env, jobject, jfloat depth)
{
	glClearDepthf( depth );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glClearStencil
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glClearStencil
  (JNIEnv *env, jobject, jint s)
{
	glClearStencil( s );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glColorMask
 * Signature: (ZZZZ)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glColorMask
  (JNIEnv *env, jobject, jboolean red, jboolean green, jboolean blue, jboolean alpha)
{
	glColorMask( red, green, blue, alpha );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCompileShader
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCompileShader
  (JNIEnv *env, jobject, jint shader)
{
	glCompileShader( shader );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCompressedTexImage2D
 * Signature: (IIIIIIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCompressedTexImage2D
  (JNIEnv *env, jobject, jint target, jint level, jint internalFormat, jint width, jint height, jint border, jint imageSize, jobject data)
{
	void* dataPtr = getDirectBufferPointer( env, data );
	glCompressedTexImage2D( target, level, internalFormat, width, height, border, imageSize, dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCompressedTexSubImage2D
 * Signature: (IIIIIIIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCompressedTexSubImage2D
  (JNIEnv *env, jobject, jint target, jint level, jint xoffset, jint yoffset, jint width, jint height, jint format, jint imageSize, jobject data)
{
	void* dataPtr = getDirectBufferPointer( env, data );
	glCompressedTexSubImage2D( target, level, xoffset, yoffset, width, height, format, imageSize, dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCopyTexImage2D
 * Signature: (IIIIIIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCopyTexImage2D
  (JNIEnv *env, jobject, jint target, jint level, jint  internalFormat, jint x, jint y, jint width, jint height, jint border)
{
	glCopyTexImage2D( target, level, internalFormat, x, y, width, height, border );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCopyTexSubImage2D
 * Signature: (IIIIIIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCopyTexSubImage2D
  (JNIEnv *env, jobject, jint target, jint level, jint xoffset, jint yoffset, jint x, jint y, jint width, jint height)
{
	glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCreateProgram
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCreateProgram
  (JNIEnv *env, jobject)
{
	return glCreateProgram( );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCreateShader
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCreateShader
  (JNIEnv *env, jobject, jint type)
{
	return glCreateShader( type );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glCullFace
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glCullFace
  (JNIEnv *env, jobject, jint mode)
{
	glCullFace( mode );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDeleteBuffers
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDeleteBuffers
  (JNIEnv *env, jobject, jint n, jobject buffers)
{
	void* dataPtr = getDirectBufferPointer( env, buffers );
	glDeleteBuffers( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDeleteFramebuffers
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDeleteFramebuffers
  (JNIEnv *env, jobject, jint n, jobject framebuffers)
{
	void* dataPtr = getDirectBufferPointer( env, framebuffers );
	glDeleteFramebuffers( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDeleteProgram
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDeleteProgram
  (JNIEnv *env, jobject, jint program)
{
	glDeleteProgram( program );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDeleteRenderbuffers
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDeleteRenderbuffers
  (JNIEnv *env, jobject, jint n, jobject renderbuffers)
{
	void* dataPtr = getDirectBufferPointer( env, renderbuffers );
	glDeleteRenderbuffers( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDeleteShader
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDeleteShader
  (JNIEnv *env, jobject, jint shader)
{
	glDeleteShader( shader );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDeleteTextures
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDeleteTextures
  (JNIEnv *env, jobject, jint n, jobject textures)
{
	void* dataPtr = getDirectBufferPointer( env, textures );
	glDeleteTextures( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDepthFunc
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDepthFunc
  (JNIEnv *env, jobject, jint func)
{
	glDepthFunc( func );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDepthMask
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDepthMask
  (JNIEnv *env, jobject, jboolean flag)
{
	glDepthMask( flag );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDepthRangef
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDepthRangef
  (JNIEnv *env, jobject, jfloat zNear, jfloat zFar)
{
	glDepthRangef( zNear, zFar );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDetachShader
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDetachShader
  (JNIEnv *env, jobject, jint program, jint shader)
{
	glDetachShader( program, shader );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDisable
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDisable
  (JNIEnv *env, jobject, jint cap)
{
	glDisable( cap );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDisableVertexAttribArray
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDisableVertexAttribArray
  (JNIEnv *env, jobject, jint index)
{
	glDisableVertexAttribArray( index );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDrawArrays
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDrawArrays
  (JNIEnv *env, jobject, jint mode, jint first, jint count)
{
	glDrawArrays( mode, first, count );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glDrawElements
 * Signature: (IIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDrawElements__IIILjava_nio_Buffer_2
(JNIEnv *env, jobject, jint mode, jint count, jint type, jobject indices)
{
	void* dataPtr = getDirectBufferPointer( env, indices );
	//__android_log_print(ANDROID_LOG_INFO, "GL2", "drawelements");
	glDrawElements( mode, count, type, dataPtr );
}

JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glDrawElements__IIII
  (JNIEnv *, jobject, jint mode, jint count, jint type, jint indices)
{
	glDrawElements( mode, count, type, (const void*)indices );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glEnable
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glEnable
  (JNIEnv *env, jobject, jint cap)
{
	glEnable( cap );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glEnableVertexAttribArray
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glEnableVertexAttribArray
  (JNIEnv *env, jobject, jint index)
{
	glEnableVertexAttribArray( index );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glFinish
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glFinish
  (JNIEnv *env, jobject)
{
	glFinish();
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glFlush
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glFlush
  (JNIEnv *env, jobject)
{
	glFlush();
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glFramebufferRenderbuffer
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glFramebufferRenderbuffer
  (JNIEnv *env, jobject, jint target, jint attachment, jint renderbuffertarget, jint renderbuffer)
{
	glFramebufferRenderbuffer( target, attachment, renderbuffertarget, renderbuffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glFramebufferTexture2D
 * Signature: (IIIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glFramebufferTexture2D
  (JNIEnv *env, jobject, jint target, jint attachment, jint textarget, jint texture, jint level)
{
	glFramebufferTexture2D( target, attachment, textarget, texture, level );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glFrontFace
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glFrontFace
  (JNIEnv *env, jobject, jint mode)
{ //XXXX
	glFrontFace( mode );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGenBuffers
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGenBuffers
  (JNIEnv *env, jobject, jint n, jobject buffers)
{
	void* dataPtr = getDirectBufferPointer( env, buffers );
	glGenBuffers( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGenerateMipmap
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGenerateMipmap
  (JNIEnv *env, jobject, jint target)
{
	glGenerateMipmap( target );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGenFramebuffers
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGenFramebuffers
  (JNIEnv *env, jobject, jint n, jobject framebuffers)
{
	void* dataPtr = getDirectBufferPointer( env, framebuffers );
	glGenFramebuffers( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGenRenderbuffers
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGenRenderbuffers
  (JNIEnv *env, jobject, jint n, jobject renderbuffers)
{
	void* dataPtr = getDirectBufferPointer( env, renderbuffers );
	glGenRenderbuffers( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGenTextures
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGenTextures
  (JNIEnv *env, jobject, jint n, jobject textures)
{
	void* dataPtr = getDirectBufferPointer( env, textures );
	glGenTextures( n, (GLuint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetActiveAttrib
 * Signature: (IIILjava/nio/Buffer;Ljava/nio/IntBuffer;Ljava/nio/Buffer;Ljava/lang/String;)V
 */
JNIEXPORT jstring JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetActiveAttrib
  (JNIEnv *env, jobject, jint program, jint index, jobject size, jobject type )
{
	// FIXME is this wrong?
	char cname[2048];
	void* sizePtr = getDirectBufferPointer( env, size );
	void* typePtr = getDirectBufferPointer( env, type );
	glGetActiveAttrib( program, index, 2048, NULL, (GLint*)sizePtr, (GLenum*)typePtr, cname );

	return env->NewStringUTF( cname );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetActiveUniform
 * Signature: (IIILjava/nio/Buffer;Ljava/nio/IntBuffer;Ljava/nio/Buffer;Ljava/lang/String;)V
 */
JNIEXPORT jstring JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetActiveUniform
  (JNIEnv *env, jobject, jint program, jint index, jobject size, jobject type)
{
	// FIXME is this wrong?
	char cname[2048];
	void* sizePtr = getDirectBufferPointer( env, size );
	void* typePtr = getDirectBufferPointer( env, type );
	glGetActiveUniform( program, index, 2048, NULL, (GLint*)sizePtr, (GLenum*)typePtr, cname );

	return env->NewStringUTF( cname );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetAttachedShaders
 * Signature: (IILjava/nio/Buffer;Ljava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetAttachedShaders
  (JNIEnv *env, jobject, jint program, jint maxcount, jobject count, jobject shaders)
{
	void* countPtr = getDirectBufferPointer( env, count );
	void* shaderPtr = getDirectBufferPointer( env, shaders );
	glGetAttachedShaders( program, maxcount, (GLsizei*)countPtr, (GLuint*)shaderPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetAttribLocation
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetAttribLocation
  (JNIEnv *env, jobject, jint program, jstring name)
{
	const char* cname = getString( env, name );
	int loc = glGetAttribLocation( program, cname );
	releaseString( env, name, cname );
	return loc;
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetBooleanv
 * Signature: (ILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetBooleanv
  (JNIEnv *env, jobject, jint program, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetBooleanv( program, (GLboolean*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetBufferParameteriv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetBufferParameteriv
  (JNIEnv *env, jobject, jint target, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetBufferParameteriv( target, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetError
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetError
  (JNIEnv *env, jobject)
{
	return glGetError();
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetFloatv
 * Signature: (ILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetFloatv
  (JNIEnv *env, jobject, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetFloatv( pname, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetFramebufferAttachmentParameteriv
 * Signature: (IIILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetFramebufferAttachmentParameteriv
  (JNIEnv *env, jobject, jint target, jint attachment, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetFramebufferAttachmentParameteriv( target, attachment, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetIntegerv
 * Signature: (ILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetIntegerv
  (JNIEnv *env, jobject, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetIntegerv( pname, (GLint*)dataPtr);
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetProgramiv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetProgramiv
  (JNIEnv *env, jobject, jint program, jint pname, jobject params)
{
	void *dataPtr = getDirectBufferPointer( env, params );
	glGetProgramiv( program, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetProgramInfoLog
 * Signature: (IILjava/nio/Buffer;Ljava/lang/String;)V
 */
JNIEXPORT jstring JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetProgramInfoLog
  (JNIEnv *env, jobject, jint program )
{
	char info[1024*10]; // FIXME 10k limit should suffice
	int length = 0;
	glGetProgramInfoLog( program, 1024*10, &length, info );
	return env->NewStringUTF( info );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetRenderbufferParameteriv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetRenderbufferParameteriv
  (JNIEnv *env, jobject, jint target, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetRenderbufferParameteriv( target, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetShaderiv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetShaderiv
  (JNIEnv *env, jobject, jint shader, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetShaderiv( shader, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetShaderInfoLog
 * Signature: (IILjava/nio/Buffer;Ljava/lang/String;)V
 */
JNIEXPORT jstring JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetShaderInfoLog
  (JNIEnv *env, jobject, jint shader )
{
	char info[1024*10]; // FIXME 10k limit should suffice
	int length = 0;
	glGetShaderInfoLog( shader, 1024*10, &length, info );
	return env->NewStringUTF( info );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetShaderPrecisionFormat
 * Signature: (IILjava/nio/IntBuffer;Ljava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetShaderPrecisionFormat
  (JNIEnv *env, jobject, jint shadertype, jint precisiontype, jobject range, jobject precision)
{
	void* rangePtr = getDirectBufferPointer( env, range );
	void* precisionPtr = getDirectBufferPointer( env, precision );
	glGetShaderPrecisionFormat( shadertype, precisiontype, (GLint*)rangePtr, (GLint*)precisionPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetShaderSource
 * Signature: (IILjava/nio/Buffer;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetShaderSource
  (JNIEnv *env, jobject, jint shader, jint bufsize, jobject length, jstring source)
{
	env->ThrowNew(UOEClass, "This method is not supported"); // FIXME won't implement this shit.
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetString
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetString
  (JNIEnv *env, jobject, jint name)
{
	const char * chars = (const char *)glGetString((GLenum)name);
	jstring output = env->NewStringUTF(chars);
	return output;
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetTexParameterfv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetTexParameterfv
  (JNIEnv *env, jobject, jint target, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetTexParameterfv( target, pname, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetTexParameteriv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetTexParameteriv
  (JNIEnv *env, jobject, jint target, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetTexParameteriv( target, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetUniformfv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetUniformfv
  (JNIEnv *env, jobject, jint program, jint location, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetUniformfv( program, location, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetUniformiv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetUniformiv
  (JNIEnv *env, jobject, jint program, jint location, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetUniformiv( program, location, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetUniformLocation
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetUniformLocation
  (JNIEnv *env, jobject, jint program, jstring name)
{
	const char* cname = getString( env, name );
	int location = glGetUniformLocation( program, cname );
	releaseString( env, name, cname );
	return location;
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetVertexAttribfv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetVertexAttribfv
  (JNIEnv *env, jobject, jint index, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetVertexAttribfv( index, pname, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetVertexAttribiv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetVertexAttribiv
  (JNIEnv *env, jobject, jint index, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glGetVertexAttribiv( index, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glGetVertexAttribPointerv
 * Signature: (IILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glGetVertexAttribPointerv
  (JNIEnv *env, jobject, jint index, jint pname, jobject pointer)
{
	env->ThrowNew(UOEClass, "This method is not supported"); // FIXME won't implement this shit
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glHint
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glHint
  (JNIEnv *env, jobject, jint target, jint mode)
{
	glHint( target, mode );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsBuffer
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsBuffer
  (JNIEnv *env, jobject, jint buffer)
{
	return glIsBuffer( buffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsEnabled
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsEnabled
  (JNIEnv *env, jobject, jint cap)
{
	return glIsEnabled( cap );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsFramebuffer
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsFramebuffer
  (JNIEnv *env, jobject, jint framebuffer)
{
	return glIsFramebuffer( framebuffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsProgram
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsProgram
  (JNIEnv *env, jobject, jint program)
{
	return glIsProgram( program );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsRenderbuffer
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsRenderbuffer
  (JNIEnv *env, jobject, jint renderbuffer)
{
	return glIsRenderbuffer( renderbuffer );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsShader
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsShader
  (JNIEnv *env, jobject, jint shader)
{
	return glIsShader( shader );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glIsTexture
 * Signature: (I)C
 */
JNIEXPORT jboolean JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glIsTexture
  (JNIEnv *env, jobject, jint texture)
{
	return glIsTexture( texture );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glLineWidth
 * Signature: (F)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glLineWidth
  (JNIEnv *env, jobject, jfloat width)
{
	glLineWidth( width );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glLinkProgram
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glLinkProgram
  (JNIEnv *env, jobject, jint program)
{
	glLinkProgram( program );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glPixelStorei
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glPixelStorei
  (JNIEnv *env, jobject, jint pname, jint param)
{
	glPixelStorei( pname, param );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glPolygonOffset
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glPolygonOffset
  (JNIEnv *env, jobject, jfloat factor, jfloat units)
{
	glPolygonOffset( factor, units );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glReadPixels
 * Signature: (IIIIIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glReadPixels
  (JNIEnv *env, jobject, jint x, jint y, jint width, jint height, jint format, jint type, jobject pixels)
{
	void* dataPtr = getDirectBufferPointer( env, pixels );
	glReadPixels( x, y, width, height, format, type, dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glReleaseShaderCompiler
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glReleaseShaderCompiler
  (JNIEnv *env, jobject)
{
	glReleaseShaderCompiler();
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glRenderbufferStorage
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glRenderbufferStorage
  (JNIEnv *env, jobject, jint target, jint internalFormat, jint width, jint height)
{
	glRenderbufferStorage( target, internalFormat, width, height );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glSampleCoverage
 * Signature: (FZ)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glSampleCoverage
  (JNIEnv *env, jobject, jfloat value, jboolean inver)
{
	glSampleCoverage( value, inver );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glScissor
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glScissor
  (JNIEnv *env, jobject, jint x, jint y, jint width, jint height)
{
	glScissor( x, y, width, height );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glShaderBinary
 * Signature: (ILjava/nio/IntBuffer;ILjava/nio/Buffer;I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glShaderBinary
  (JNIEnv *env, jobject, jint n, jobject shaders, jint binaryformat, jobject binary, jint length)
{
	void* shaderPtr = getDirectBufferPointer( env, shaders );
	void* binaryPtr = getDirectBufferPointer( env, binary );
	glShaderBinary( n, (GLuint*)shaderPtr, binaryformat, binaryPtr, length );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glShaderSource
 * Signature: (IILjava/lang/String;Ljava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glShaderSource
  (JNIEnv *env, jobject, jint shader, jstring string )
{
	const char* cstring = getString( env, string );
	glShaderSource( shader, 1, &cstring, NULL );
	releaseString( env, string, cstring );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glStencilFunc
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glStencilFunc
  (JNIEnv *env, jobject, jint func, jint ref, jint mask)
{
	glStencilFunc( func, ref, mask );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glStencilFuncSeparate
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glStencilFuncSeparate
  (JNIEnv *env, jobject, jint face, jint func, jint ref, jint mask)
{
	glStencilFuncSeparate( face, func, ref, mask );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glStencilMask
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glStencilMask
  (JNIEnv *env, jobject, jint mask)
{
	glStencilMask( mask );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glStencilMaskSeparate
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glStencilMaskSeparate
  (JNIEnv *env, jobject, jint face, jint mask)
{
	glStencilMaskSeparate( face, mask );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glStencilOp
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glStencilOp
  (JNIEnv *env, jobject, jint fail, jint zFail, jint zpass)
{
	glStencilOp( fail, zFail, zpass );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glStencilOpSeparate
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glStencilOpSeparate
  (JNIEnv *env, jobject, jint face, jint fail, jint zFail, jint zPass)
{
	glStencilOpSeparate( face, fail, zFail, zPass );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glTexImage2D
 * Signature: (IIIIIIIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glTexImage2D
  (JNIEnv *env, jobject, jint target, jint level, jint internalformat, jint width, jint height, jint border, jint format, jint type, jobject pixels)
{
	void* dataPtr = getDirectBufferPointer( env, pixels );
	glTexImage2D( target, level, internalformat, width, height, border, format, type, dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glTexParameterf
 * Signature: (IIF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glTexParameterf
  (JNIEnv *env, jobject, jint target, jint pname, jfloat param)
{
	glTexParameterf( target, pname, param );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glTexParameterfv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glTexParameterfv
  (JNIEnv *env, jobject, jint target, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glTexParameterfv( target, pname, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glTexParameteri
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glTexParameteri
  (JNIEnv *env, jobject, jint target, jint pname, jint param)
{
	glTexParameteri( target, pname, param );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glTexParameteriv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glTexParameteriv
  (JNIEnv *env, jobject, jint target, jint pname, jobject params)
{
	void* dataPtr = getDirectBufferPointer( env, params );
	glTexParameteriv( target, pname, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glTexSubImage2D
 * Signature: (IIIIIIIILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glTexSubImage2D
  (JNIEnv *env, jobject, jint target, jint level, jint xoffset, jint yoffset, jint width, jint height, jint format, jint type, jobject pixels)
{
	void* dataPtr = getDirectBufferPointer( env, pixels );
	glTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform1f
 * Signature: (IF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform1f
  (JNIEnv *env, jobject, jint location, jfloat x)
{
	glUniform1f( location, x );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform1fv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform1fv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform1fv( location, count, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform1i
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform1i
  (JNIEnv *env, jobject, jint location, jint x)
{
	glUniform1i( location, x );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform1iv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform1iv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform1iv( location, count, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform2f
 * Signature: (IFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform2f
  (JNIEnv *env, jobject, jint location, jfloat x, jfloat y)
{
	glUniform2f( location, x, y );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform2fv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform2fv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform2fv( location, count, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform2i
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform2i
  (JNIEnv *env, jobject, jint location, jint x, jint y)
{
	glUniform2i( location, x, y );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform2iv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform2iv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform2iv( location, count, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform3f
 * Signature: (IFFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform3f
  (JNIEnv *env, jobject, jint location, jfloat x, jfloat y, jfloat z)
{
	glUniform3f( location, x, y, z );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform3fv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform3fv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform3fv( location, count, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform3i
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform3i
  (JNIEnv *env, jobject, jint location, jint x, jint y, jint z)
{
	glUniform3i( location, x, y, z );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform3iv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform3iv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform3iv( location, count, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform4f
 * Signature: (IFFFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform4f
  (JNIEnv *env, jobject, jint location, jfloat x, jfloat y, jfloat z, jfloat w)
{
	glUniform4f( location, x, y, z, w );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform4fv
 * Signature: (IILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform4fv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform4fv( location, count, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform4i
 * Signature: (IIIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform4i
  (JNIEnv *env, jobject, jint location, jint x, jint y, jint z, jint w)
{
	glUniform4i( location, x, y, z, w );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniform4iv
 * Signature: (IILjava/nio/IntBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniform4iv
  (JNIEnv *env, jobject, jint location, jint count, jobject v)
{
	void* dataPtr = getDirectBufferPointer( env, v );
	glUniform4iv( location, count, (GLint*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniformMatrix2fv
 * Signature: (IIZLjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniformMatrix2fv
  (JNIEnv *env, jobject, jint location, jint count, jboolean transpose, jobject value)
{
	void* dataPtr = getDirectBufferPointer( env, value );
	glUniformMatrix2fv( location, count, transpose, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniformMatrix3fv
 * Signature: (IIZLjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniformMatrix3fv
  (JNIEnv *env, jobject, jint location, jint count, jboolean transpose, jobject value)
{
	void* dataPtr = getDirectBufferPointer( env, value );
	glUniformMatrix3fv( location, count, transpose, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUniformMatrix4fv
 * Signature: (IIZLjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUniformMatrix4fv
  (JNIEnv *env, jobject, jint location, jint count, jboolean transpose, jobject value)
{
	void* dataPtr = getDirectBufferPointer( env, value );
	glUniformMatrix4fv( location, count, transpose, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glUseProgram
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glUseProgram
  (JNIEnv *env, jobject, jint program)
{
	glUseProgram( program );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glValidateProgram
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glValidateProgram
  (JNIEnv *env, jobject, jint program)
{
	glValidateProgram( program );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib1f
 * Signature: (IF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib1f
  (JNIEnv *env, jobject, jint indx, jfloat x)
{
	glVertexAttrib1f( indx, x );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib1fv
 * Signature: (ILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib1fv
  (JNIEnv *env, jobject, jint indx, jobject values)
{
	void* dataPtr = getDirectBufferPointer( env, values );
	glVertexAttrib1fv( indx, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib2f
 * Signature: (IFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib2f
  (JNIEnv *env, jobject, jint indx, jfloat x, jfloat y)
{
	glVertexAttrib2f( indx, x, y );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib2fv
 * Signature: (ILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib2fv
  (JNIEnv *env, jobject, jint indx, jobject values)
{
	void* dataPtr = getDirectBufferPointer( env, values );
	glVertexAttrib2fv( indx, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib3f
 * Signature: (IFFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib3f
  (JNIEnv *env, jobject, jint indx, jfloat x, jfloat y, jfloat z)
{
	glVertexAttrib3f( indx, x, y, z );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib3fv
 * Signature: (ILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib3fv
  (JNIEnv *env, jobject, jint indx, jobject values)
{
	void* dataPtr = getDirectBufferPointer( env, values );
	glVertexAttrib3fv( indx, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib4f
 * Signature: (IFFFF)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib4f
  (JNIEnv *env, jobject, jint indx, jfloat x, jfloat y, jfloat z, jfloat w)
{
	glVertexAttrib4f( indx, x, y, z, w );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttrib4fv
 * Signature: (ILjava/nio/FloatBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttrib4fv
  (JNIEnv *env, jobject, jint indx, jobject values)
{
	void* dataPtr = getDirectBufferPointer( env, values );
	glVertexAttrib4fv( indx, (GLfloat*)dataPtr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glVertexAttribPointer
 * Signature: (IIIZILjava/nio/Buffer;)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttribPointer__IIIZILjava_nio_Buffer_2
  (JNIEnv *env, jobject, jint indx, jint size, jint type, jboolean normalized, jint stride, jobject ptr)
{
	void* dataPtr = getDirectBufferPointer( env, ptr );
	glVertexAttribPointer( indx, size, type, normalized, stride, dataPtr );
}

JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glVertexAttribPointer__IIIZII
  (JNIEnv *, jobject, jint indx, jint size, jint type, jboolean normalized, jint stride, jint ptr)
{
	glVertexAttribPointer( indx, size, type, normalized, stride, (const void*)ptr );
}

/*
 * Class:     com_badlogic_gdx_backends_android_AndroidGL20
 * Method:    glViewport
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_badlogic_gdx_backends_android_AndroidGL20_glViewport
  (JNIEnv *env, jobject, jint x, jint y, jint width, jint height)
{
	glViewport( x, y, width, height );
}
