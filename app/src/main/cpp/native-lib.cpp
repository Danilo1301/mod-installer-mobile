#include "pch.h"
#include "app.h"

jint JNI_OnLoad(JavaVM* vm, void*)
{
    _javaVM = vm;

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_com_daniloszk_mod_1installer_ConsoleActivity_registerConsoleActivity(
    JNIEnv* env,
    jobject activity)
{
    //if (_consoleActivity) env->DeleteGlobalRef(_consoleActivity);

    _consoleActivity = env->NewGlobalRef(activity);

    jclass activityClass = env->GetObjectClass(activity);

    _addConsoleLine = env->GetMethodID(activityClass,"addConsoleLine","(ILjava/lang/String;I)V");
    _setConsoleLine = env->GetMethodID(activityClass,"setConsoleLine","(ILjava/lang/String;)V");
    _moveGameFiles = env->GetMethodID(activityClass,"moveGameFiles","(Ljava/lang/String;[Ljava/lang/String;)V");
    _getFilesRecursive = env->GetMethodID(activityClass,"getFilesRecursive","(Ljava/lang/String;)[Ljava/lang/String;");
    _showUserChoice = env->GetMethodID(activityClass,"showUserChoice","(I[Ljava/lang/String;)V");
    _backupFiles = env->GetMethodID(activityClass,"backupFiles","([Ljava/lang/String;I)V");
    _createFolder = env->GetMethodID(activityClass, "createFolder", "(Ljava/lang/String;Ljava/lang/String;)Z");
    _clearCacheFolder = env->GetMethodID(activityClass,"clearCacheFolder","()V");

    env->DeleteLocalRef(activityClass);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_daniloszk_mod_1installer_ConsoleActivity_installMod(
        JNIEnv* env,
        jobject /* this */,
        jstring url,
        jstring folderUri,
        jstring cachePath)
{
    const char* urlChars = env->GetStringUTFChars(url, nullptr);
    const char* folderUriChars = env->GetStringUTFChars(folderUri, nullptr);
    const char* cachePathChars = env->GetStringUTFChars(cachePath, nullptr);

    std::string modUrl = urlChars;
    std::string gameFolderUri = folderUriChars;
    std::string cacheDirectory = cachePathChars;

    env->ReleaseStringUTFChars(url, urlChars);
    env->ReleaseStringUTFChars(folderUri, folderUriChars);
    env->ReleaseStringUTFChars(cachePath, cachePathChars);

    Paths::CachePath = cachePathChars;
    Paths::GameFolderPath = gameFolderUri;

    App::Reset();
    App::InstallApp(modUrl);

    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_daniloszk_mod_1installer_ConsoleActivity_onUserChoice(
        JNIEnv*,
        jobject,
        jint value)
{
    App::OnUserChoice(
            static_cast<int>(value)
    );
}

extern "C" JNIEXPORT void JNICALL
Java_com_daniloszk_mod_1installer_ConsoleActivity_onCallbackReceived(
        JNIEnv*,
        jobject,
        jint callbackId)
{
    App::OnCallbackReceived(static_cast<int>(callbackId));
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_daniloszk_mod_1installer_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */)
{
    std::string hello = "Hello from C++, and edited";

    return env->NewStringUTF(hello.c_str());
}