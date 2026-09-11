#include "console.h"

int lastConsoleMessageIdUsed = 1;

int Console::AddConsoleLine(const std::string& text)
{
    return AddConsoleLine(text, 0xFFFFFFFF);
}

int Console::AddConsoleLine(const std::string& text, int color)
{
    lastConsoleMessageIdUsed += 1;
    int id = lastConsoleMessageIdUsed;

    LOGI("AddConsoleLine: id=%d text=%s", id, text.c_str());

    if (!_consoleActivity)
    {
        LOGI("AddConsoleLine: _consoleActivity is null");
        return -1;
    }

    if (!_addConsoleLine)
    {
        LOGI("AddConsoleLine: _addConsoleLine is null");
        return -1;
    }

    if (!_javaVM)
    {
        LOGI("AddConsoleLine: _javaVM is null");
        return -1;
    }

    JNIEnv* env = nullptr;
    bool attached = false;

    jint envResult =
            _javaVM->GetEnv(
                    reinterpret_cast<void**>(&env),
                    JNI_VERSION_1_6);


    if (envResult != JNI_OK)
    {
        if (_javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            return -1;
        }

        attached = true;
    }

    jstring jText = env->NewStringUTF(text.c_str());

    if (!jText)
    {
        LOGI("AddConsoleLine: NewStringUTF failed");

        if (attached)
            _javaVM->DetachCurrentThread();

        return -1;
    }

    env->CallVoidMethod(
            _consoleActivity,
            _addConsoleLine,
            id,
            jText,
            color);

    if (env->ExceptionCheck())
    {
        LOGI("AddConsoleLine: Java exception!");

        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    else
    {
    }

    env->DeleteLocalRef(jText);

    if (attached)
        _javaVM->DetachCurrentThread();

    return id;
}

void Console::SetConsoleLine(
        int id,
        const std::string& text)
{
    LOGI("SetConsoleLine: id=%d text=%s", id, text.c_str());

    if (!_consoleActivity)
    {
        LOGI("SetConsoleLine: _consoleActivity is null");
        return;
    }

    if (!_setConsoleLine)
    {
        LOGI("SetConsoleLine: _setConsoleLine is null");
        return;
    }

    if (!_javaVM)
    {
        LOGI("SetConsoleLine: _javaVM is null");
        return;
    }

    JNIEnv* env = nullptr;
    bool attached = false;

    jint envResult =
            _javaVM->GetEnv(
                    reinterpret_cast<void**>(&env),
                    JNI_VERSION_1_6);

    if (envResult != JNI_OK)
    {
        if (_javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            return;
        }

        attached = true;
    }

    jstring jText = env->NewStringUTF(text.c_str());

    if (!jText)
    {
        LOGI("SetConsoleLine: NewStringUTF failed");

        if (attached)
            _javaVM->DetachCurrentThread();

        return;
    }

    env->CallVoidMethod(
            _consoleActivity,
            _setConsoleLine,
            id,
            jText);

    if (env->ExceptionCheck())
    {
        LOGI("SetConsoleLine: Java exception!");

        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    else
    {
    }

    env->DeleteLocalRef(jText);

    if (attached)
        _javaVM->DetachCurrentThread();
}