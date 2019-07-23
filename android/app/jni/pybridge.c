/**
    This file defines the JNI implementation of the PyBridge class.

    It implements the native methods of the class and makes sure that
    all the prints and errors from the Python interpreter is redirected
    to the Android log. This is specially useful as it allows us to
    debug the Python code running on the Android device using logcat.

*/

#include <Python.h>
#include <jni.h>
#include <android/log.h>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <assert.h>

// Android log function wrappers
static const char* kTAG = "hello-jniCallback";



#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

#define LOG(x) __android_log_write(ANDROID_LOG_INFO, "pybridge", (x))


JavaVM  *jvm;
jobject myObj;
jclass jniHelperClz;

void newLineStdOut(char *str)
{
    JavaVM *javaVM = jvm;
    JNIEnv *env;
    jint res = (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_6);

    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    // Construct a String
    jstring jstr = (*env)->NewStringUTF(env, str);
    jmethodID memFunc = (*env)->GetMethodID(env, jniHelperClz,
                                            "messageMe", "(Ljava/lang/String;)V");
    if (!memFunc) {
         LOGE("Failed to retrieve getRuntimeMemorySize() methodID @ line %d",
        __LINE__);
        return;
    }
    (*env)->CallVoidMethod(env, myObj, memFunc, jstr);
}

jstring getStdIn()
{

    JavaVM *javaVM = jvm;
    JNIEnv *env;
    jint res = (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_6);

    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    // Construct a String
    jmethodID memFunc = (*env)->GetMethodID(env, jniHelperClz,
                                            "inputGet", "()Ljava/lang/String;");
    if (!memFunc) {
         LOGE("Failed to retrieve getRuntimeMemorySize() methodID @ line %d",
        __LINE__);
        return;
    }
    jobject result = (*env)->CallObjectMethod(env, myObj, memFunc);

    const char* str = (*env)->GetStringUTFChars(env,(jstring) result, NULL); // should be released but what a heck, it's a tutorial :)
     LOGE("getInput %s",
            str);

    return (*env)->NewStringUTF(env, str);
}

/* --------------- */
/*   Android log   */
/* --------------- */

static PyObject *androidlog(PyObject *self, PyObject *args)
{
    char *str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    newLineStdOut(str);
    Py_RETURN_NONE;
}

static PyObject *androidget(PyObject *self)
{

    JavaVM *javaVM = jvm;
    JNIEnv *env;
    jint res = (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_6);

    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    jstring sts = getStdIn();

    const char *nativeString = (*env)->GetStringUTFChars(env, sts, 0);

    return PyUnicode_FromString(nativeString);
}


static PyMethodDef AndroidlogMethods[] = {
    {"log", androidlog, METH_VARARGS, "Logs to Android stdout"},
    {"get", androidget, METH_VARARGS, "Get from Android stdin"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef AndroidlogModule = {
    PyModuleDef_HEAD_INIT,
    "androidlog",        /* m_name */
    "Log for Android",   /* m_doc */
    -1,                  /* m_size */
    AndroidlogMethods    /* m_methods */
};


PyMODINIT_FUNC PyInit_androidlog(void)
{
    return PyModule_Create(&AndroidlogModule);
}


void setAndroidLog()
{
    // Inject  bootstrap code to redirect python stdin/stdout
    // to the androidlog module
    PyRun_SimpleString(
            "import sys, time\n" \
            "import androidlog\n" \
            "class LogFile(object):\n" \
            "    def __init__(self):\n" \
            "        self.buffer = ''\n" \
            "    def write(self, s):\n" \
            "        s = self.buffer + s\n" \
            "        lines = s.split(\"\\n\")\n" \
            "        for l in lines[:-1]:\n" \
            "            androidlog.log(l)\n" \
            "        self.buffer = lines[-1]\n" \
            "    def readline(self):\n"\
            "        done = ' '\n"\
            "        while True:\n"\
            "            done = androidlog.get()\n"\
            "            if ':ok' in done:\n"\
            "                break\n"\
            "            else:\n"\
            "                time.sleep(0.3)\n"\
            "        return done.replace(':ok','')\n"\
            "    def flush(self):\n" \
            "        return\n" \
            "sys.stdin = sys.stdout = sys.stderr = LogFile()\n"
    );
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* aReserved)
{
     JNIEnv* env;

     // cache java VM
     jvm = vm;

     if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
             return JNI_ERR; // JNI version not supported.
     }

     return JNI_VERSION_1_6;
}


/* ------------------ */
/*   Native methods   */
/* ------------------ */

/**
    This function configures the location of the standard library,
    initializes the interpreter and sets up the python log redirect.
    It runs a file called bootstrap.py before returning, so make sure
    that you configure all your python code on that file.

    Note: the function must receives a string with the location of the
    python files extracted from the assets folder.

*/

//com.learnprogramming.codecamp.python.interpreter
//flutterdesign.learning.com.flutter_python_2.interpreter;
//com_learnprogramming_codecamp_python_interpreter
//flutterdesign.learning.com.flutterpy.interpreter
JNIEXPORT jint JNICALL Java_flutterdesign_learning_com_flutterpy_interpreter_PyBridge_start
        (JNIEnv *env, jobject instance, jstring path)
{
    LOG("Initializing the Python interpreter");

    myObj = (*env)->NewGlobalRef(env, instance);

    jclass  clz = (*env)->FindClass(env,
                                        "flutterdesign/learning/com/flutterpy/interpreter/PyBridge");
    jniHelperClz = (*env)->NewGlobalRef(env, clz);
    //com.learnprogramming.codecamp.python.interpreter

    // Get the location of the python files
    const char *pypath = (*env)->GetStringUTFChars(env, path, NULL);

    // Build paths for the Python interpreter
    char paths[512];
    snprintf(paths, sizeof(paths), "%s:%s/stdlib.zip", pypath, pypath);

    // Set Python paths
    wchar_t *wchar_paths = Py_DecodeLocale(paths, NULL);
    Py_SetPath(wchar_paths);

    // Initialize Python interpreter and logging
    PyImport_AppendInittab("androidlog", PyInit_androidlog);
    Py_Initialize();
    setAndroidLog();

    // Bootstrap
    PyRun_SimpleString("import bootstrap");

    // Cleanup
    (*env)->ReleaseStringUTFChars(env, path, pypath);
    PyMem_RawFree(wchar_paths);

    return 0;
}


JNIEXPORT jint JNICALL Java_flutterdesign_learning_com_flutterpy_interpreter_PyBridge_stop
        (JNIEnv *env, jclass jc)
{
    LOG("Finalizing the Python interpreter");
    Py_Finalize();
    return 0;
}


/**
    This function is responsible for receiving a payload string
    and sending it to the router function defined in the bootstrap.py
    file.

*/
JNIEXPORT jint JNICALL Java_flutterdesign_learning_com_flutterpy_interpreter_PyBridge_call
        (JNIEnv *env, jobject instance, jstring payload)
{
    LOG("Call into Python interpreter");

    // Get the payload string
    jboolean iscopy;
    const char *payload_utf = (*env)->GetStringUTFChars(env, payload, &iscopy);

    //running
    PyRun_SimpleString(payload_utf);

    // Cleanup
    (*env)->ReleaseStringUTFChars(env, payload, payload_utf);


    return 0;
}
