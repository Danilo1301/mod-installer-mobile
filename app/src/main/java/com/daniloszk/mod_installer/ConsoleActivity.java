package com.daniloszk.mod_installer;

import android.app.Dialog;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;

public class ConsoleActivity extends AppCompatActivity
{
    private final int NO_ID = -1;

    private LinearLayout _messagesLayout;

    private ScrollView _scrollView;

    private final List<TextView> _messages = new ArrayList<>();

    private final Map<Integer, TextView> _messagesMap = new HashMap<>();

    private final Queue<Runnable> _actions = new LinkedList<>();

    private String _gameFolderUri;

    private String _cachePath;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_console);

        _messagesLayout = findViewById(R.id.console_messages);
        _scrollView = findViewById(R.id.console_scroll);

        registerConsoleActivity();

        File downloads = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_DOWNLOADS
        );

        final String url = getIntent().getStringExtra("url");
        final String gameFolderUri = getIntent().getStringExtra("game_folder_uri");
        //final String cachePath = getCacheDir().getAbsolutePath();
        final String cachePath = downloads.getAbsolutePath() + "/cache/";

        _gameFolderUri = gameFolderUri;
        _cachePath = cachePath;

        //AddMessage("Calling cpp...");
        AddMessage("Cache: " + cachePath);

        clearCacheFolder();

        new Thread(() ->
        {
            installMod(
                    url,
                    gameFolderUri,
                    cachePath
            );
        }).start();
    }

    public void AddMessage(String text) {
        addConsoleLine(NO_ID, text, Color.WHITE);
    }

    public void AddMessage(String text, int color)
    {
        addConsoleLine(NO_ID, text, color);
    }

    private void AddAction(Runnable action)
    {
        synchronized (_actions)
        {
            _actions.add(action);
        }

        runOnUiThread(this::ProcessActions);
    }

    private void ProcessActions()
    {
        while (true)
        {
            Runnable action;

            synchronized (_actions)
            {
                action = _actions.poll();
            }

            if (action == null)
                return;

            action.run();
        }
    }

    private native void registerConsoleActivity();

    public void clearCacheFolder()
    {
        File cacheFolder = new File(_cachePath);

        deleteRecursive(cacheFolder);

        cacheFolder.mkdirs();
    }

    private void deleteRecursive(File file)
    {
        if (file.isDirectory())
        {
            File[] files = file.listFiles();

            if (files != null)
            {
                for (File child : files)
                    deleteRecursive(child);
            }
        }

        file.delete();
    }

    private native boolean installMod(String url, String gameFolderUri, String cachePath);

    public void addConsoleLine(int id, String text, int color)
    {
        AddAction(() ->
        {
            TextView label = new TextView(this);

            label.setText(text);
            label.setTextSize(14);
            label.setTextColor(color);
            label.setPadding(8, 4, 8, 4);

            _messagesLayout.addView(label);

            _messages.add(label);

            if (id != NO_ID)
                _messagesMap.put(id, label);

            _scrollView.post(() ->
            {
                _scrollView.fullScroll(View.FOCUS_DOWN);
            });
        });
    }

    public void setConsoleLine(int id, String text)
    {
        if (id == NO_ID)
            return;

        AddAction(() ->
        {
            TextView label = _messagesMap.get(id);

            if (label != null)
                label.setText(text);
        });
    }

    public void moveGameFiles(
            String newGameFilesPath,
            String[] deleteFiles)
    {
        new Thread(() ->
        {
            try
            {
                AddMessage("Starting file installation...");

                Uri gameFolderUri =
                        Uri.parse(_gameFolderUri);

                DocumentFile gameFolder =
                        DocumentFile.fromTreeUri(
                                this,
                                gameFolderUri
                        );

                if (gameFolder == null ||
                        !gameFolder.isDirectory())
                {
                    AddMessage("Invalid game folder");
                    return;
                }

                File sourceFolder =
                        new File(newGameFilesPath);

                if (!sourceFolder.exists() ||
                        !sourceFolder.isDirectory())
                {
                    AddMessage(
                            "newGameFiles folder does not exist"
                    );

                    return;
                }

                int deletedCount = 0;
                int deleteMessageId = 50009;

                addConsoleLine(deleteMessageId, "Deleting...", 0xFFFFFFFF);

                for (String filePath : deleteFiles)
                {
                    setConsoleLine(deleteMessageId,  "Deleting: " + filePath);

                    boolean result =
                            DeleteSafFile(filePath);

                    if (result)
                    {
                        deletedCount++;
                        setConsoleLine(deleteMessageId,  "Deleted: " + filePath);
                    }
                    else
                    {
                        AddMessage(
                                "Failed to delete: " +
                                        filePath
                        );
                    }
                }

                AddMessage(
                        "Deleted " +
                                deletedCount +
                                "/" +
                                deleteFiles.length +
                                " file(s)."
                );

                AddMessage("Moving game files...");

                long startTime =
                        System.currentTimeMillis();

                CopyDirectoryToSaf(
                        sourceFolder,
                        gameFolder
                );

                long elapsed =
                        System.currentTimeMillis() -
                                startTime;

                AddMessage(
                        "Game files moved successfully."
                );

                AddMessage(
                        "Time: " +
                                (elapsed / 1000.0) +
                                " seconds"
                );

                onCallbackReceived(50028);
            }
            catch (Exception e)
            {
                AddMessage(
                        "Failed to move game files: " +
                                e.getMessage()
                );

                e.printStackTrace();
            }
        }).start();
    }

    private void CopyDirectoryToSaf(
            File source,
            DocumentFile destination)
    {
        int messageId = 50010;

        addConsoleLine(
                messageId,
                "Moving game files...",
                0xFFFFFFFF
        );

        CopyDirectoryToSaf(
                source,
                destination,
                messageId
        );

        setConsoleLine(
                messageId,
                "Game files moved."
        );
    }

    private void CopyDirectoryToSaf(
            File source,
            DocumentFile destination,
            int messageId)
    {
        File[] files = source.listFiles();

        if (files == null)
        {
            setConsoleLine(
                    messageId,
                    "Failed to list: " +
                            source.getName()
            );

            return;
        }

        for (File file : files)
        {
            if (file.isDirectory())
            {
                setConsoleLine(
                        messageId,
                        "Entering: " +
                                file.getName()
                );

                DocumentFile directory =
                        destination.findFile(
                                file.getName()
                        );

                if (directory == null)
                {
                    directory =
                            destination.createDirectory(
                                    file.getName()
                            );
                }

                if (directory == null)
                {
                    setConsoleLine(
                            messageId,
                            "Failed to create: " +
                                    file.getName()
                    );

                    continue;
                }

                CopyDirectoryToSaf(
                        file,
                        directory,
                        messageId
                );
            }
            else
            {
                setConsoleLine(
                        messageId,
                        "Copying: " +
                                file.getName()
                );

                DocumentFile target =
                        destination.findFile(
                                file.getName()
                        );

                if (target != null)
                {
                    setConsoleLine(
                            messageId,
                            "Replacing: " +
                                    file.getName()
                    );

                    if (!target.delete())
                    {
                        setConsoleLine(
                                messageId,
                                "Failed to delete: " +
                                        file.getName()
                        );

                        continue;
                    }
                }

                target =
                        destination.createFile(
                                "application/octet-stream",
                                file.getName()
                        );

                if (target == null)
                {
                    setConsoleLine(
                            messageId,
                            "Failed to create: " +
                                    file.getName()
                    );

                    continue;
                }

                try (
                        InputStream input =
                                new FileInputStream(file);

                        OutputStream output =
                                getContentResolver().openOutputStream(
                                        target.getUri()
                                )
                )
                {
                    if (output == null)
                    {
                        setConsoleLine(
                                messageId,
                                "Failed to open: " +
                                        file.getName()
                        );

                        continue;
                    }

                    byte[] buffer =
                            new byte[8192];

                    int length;

                    while ((length =
                            input.read(buffer)) != -1)
                    {
                        output.write(
                                buffer,
                                0,
                                length
                        );
                    }

                    output.flush();

                    setConsoleLine(
                            messageId,
                            "Copied: " +
                                    file.getName()
                    );
                }
                catch (IOException e)
                {
                    setConsoleLine(
                            messageId,
                            "Failed: " +
                                    file.getName() +
                                    " - " +
                                    e.getMessage()
                    );
                }
            }
        }
    }

    public String[] getFilesRecursive(String relativePath)
    {
        Uri gameFolderUri =
                Uri.parse(_gameFolderUri);

        DocumentFile gameFolder =
                DocumentFile.fromTreeUri(
                        this,
                        gameFolderUri
                );

        if (gameFolder == null || !gameFolder.isDirectory())
            return new String[0];

        String[] parts =
                relativePath.split("/");

        DocumentFile current =
                gameFolder;

        for (String part : parts)
        {
            if (part.isEmpty())
                continue;

            DocumentFile next =
                    current.findFile(part);

            if (next == null || !next.isDirectory())
                return new String[0];

            current = next;
        }

        ArrayList<String> files =
                new ArrayList<>();

        GetFilesRecursive(
                current,
                files
        );

        return files.toArray(
                new String[0]
        );
    }

    private void GetFilesRecursive(
            DocumentFile directory,
            ArrayList<String> files)
    {
        DocumentFile[] children =
                directory.listFiles();

        for (DocumentFile child : children)
        {
            if (child.isDirectory())
            {
                GetFilesRecursive(
                        child,
                        files
                );
            }
            else
            {
                files.add(
                        child.getUri().toString()
                );
            }
        }
    }

    private boolean DeleteSafFile(String fullPath)
    {
        try
        {
            Uri uri =
                    Uri.parse(fullPath);

            DocumentFile file =
                    DocumentFile.fromSingleUri(
                            this,
                            uri
                    );

            if (file == null)
                return false;

            return file.delete();
        }
        catch (Exception e)
        {
            AddMessage(
                    "Failed to delete " +
                            fullPath +
                            ": " +
                            e.getMessage()
            );

            return false;
        }
    }

    public void showUserChoice(int type, String[] lines)
    {
        runOnUiThread(() ->
        {
            if (type == 0)
            {
                ShowBackupMenu(lines);
            }
        });
    }

    private native void onUserChoice(int value);

    private void ShowBackupMenu(String[] lines)
    {
        LinearLayout layout =
                new LinearLayout(this);

        layout.setOrientation(
                LinearLayout.VERTICAL
        );

        TextView message =
                new TextView(this);

        message.setText(
                "Deseja fazer backup dos arquivos que serão deletados?"
        );

        message.setTextSize(18);
        message.setPadding(32, 24, 32, 16);

        layout.addView(message);

        ScrollView scrollView = new ScrollView(this);

        LinearLayout filesLayout =
                new LinearLayout(this);

        filesLayout.setOrientation(
                LinearLayout.VERTICAL
        );

        for (String line : lines)
        {
            TextView file =
                    new TextView(this);

            file.setText(line);
            file.setTextSize(14);
            file.setPadding(32, 4, 32, 4);

            filesLayout.addView(file);
        }

        scrollView.addView(filesLayout);

        layout.addView(
                scrollView,
                new LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.MATCH_PARENT,
                        0,
                        1
                )
        );

        LinearLayout buttons =
                new LinearLayout(this);

        buttons.setOrientation(
                LinearLayout.HORIZONTAL
        );

        Button noButton = new Button(this);

        noButton.setText("NÃO");

        Button yesButton =
                new Button(this);

        yesButton.setText("SIM");

        buttons.addView(
                noButton,
                new LinearLayout.LayoutParams(
                        0,
                        LinearLayout.LayoutParams.WRAP_CONTENT,
                        1
                )
        );

        buttons.addView(
                yesButton,
                new LinearLayout.LayoutParams(
                        0,
                        LinearLayout.LayoutParams.WRAP_CONTENT,
                        1
                )
        );

        layout.addView(buttons);

        Dialog dialog =
                new Dialog(this);

        dialog.setContentView(layout);

        Window window =
                dialog.getWindow();

        if (window != null)
        {
            window.setLayout(
                    WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.WRAP_CONTENT
            );
        }

        noButton.setOnClickListener(v ->
        {
            dialog.dismiss();
            onUserChoice(0);
        });

        yesButton.setOnClickListener(v ->
        {
            dialog.dismiss();
            onUserChoice(1);
        });

        dialog.show();

        if (window != null)
        {
            window.setLayout(
                    WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.WRAP_CONTENT
            );
        }
    }

    public void backupFiles(String[] files, int callbackId)
    {
        new Thread(() ->
        {
            AddMessage("Fazendo backup...");

            Uri gameFolderUri =
                    Uri.parse(_gameFolderUri);

            DocumentFile gameFolder =
                    DocumentFile.fromTreeUri(
                            this,
                            gameFolderUri
                    );

            if (gameFolder == null || !gameFolder.isDirectory())
            {
                AddMessage("Pasta do jogo inválida");
                onCallbackReceived(callbackId);
                return;
            }

            DocumentFile backupFolder =
                    gameFolder.findFile("backup");

            if (backupFolder == null)
            {
                backupFolder =
                        gameFolder.createDirectory("backup");
            }

            if (backupFolder == null)
            {
                AddMessage("Falha ao criar pasta de backup");
                onCallbackReceived(callbackId);
                return;
            }

            int backupCount = 0;

            for (String filePath : files)
            {
                BackupFile(
                        filePath,
                        backupFolder
                );

                    backupCount++;

            }

            AddMessage(
                    "Backup concluído! " +
                            backupCount +
                            " arquivo(s).",
                    0xFFFFA970
            );

            AddMessage(
                    "Pasta: " +
                            backupFolder.getUri(),
                    0xFFFFA970
            );

            onCallbackReceived(callbackId);
        }).start();
    }

    private native void onCallbackReceived(int callbackId);

    private void BackupFile(
            String filePath,
            DocumentFile backupFolder)
    {
        try
        {
            DocumentFile source;

            if (filePath.startsWith("content://"))
            {
                source =
                        DocumentFile.fromSingleUri(
                                this,
                                Uri.parse(filePath)
                        );
            }
            else
            {
                Uri gameFolderUri =
                        Uri.parse(_gameFolderUri);

                DocumentFile gameFolder =
                        DocumentFile.fromTreeUri(
                                this,
                                gameFolderUri
                        );

                if (gameFolder == null)
                    return;

                String[] parts =
                        filePath.split("/");

                DocumentFile current =
                        gameFolder;

                for (int i = 0; i < parts.length; i++)
                {
                    DocumentFile next =
                            current.findFile(parts[i]);

                    if (next == null)
                        return;

                    current = next;
                }

                source = current;
            }

            if (source == null || !source.isFile())
                return;

            String relativePath = filePath;

            if (filePath.startsWith("content://"))
            {
                String gamePath =
                        Uri.decode(_gameFolderUri);

                String filePathDecoded =
                        Uri.decode(filePath);

                int treeIndex =
                        gamePath.indexOf("/tree/");

                int documentIndex =
                        filePathDecoded.indexOf("/document/");

                if (treeIndex == -1 || documentIndex == -1)
                    return;

                gamePath =
                        gamePath.substring(treeIndex + 6);

                String gameFilePath =
                        filePathDecoded.substring(
                                documentIndex + 10
                        );

                if (!gameFilePath.startsWith(gamePath))
                    return;

                relativePath =
                        gameFilePath.substring(
                                gamePath.length()
                        );

                if (relativePath.startsWith("/"))
                    relativePath =
                            relativePath.substring(1);
            }

            String[] parts =
                    relativePath.split("/");

            DocumentFile currentFolder =
                    backupFolder;

            for (int i = 0; i < parts.length - 1; i++)
            {
                DocumentFile next =
                        currentFolder.findFile(parts[i]);

                if (next == null)
                {
                    next =
                            currentFolder.createDirectory(parts[i]);
                }

                if (next == null)
                    return;

                currentFolder = next;
            }

            String fileName =
                    parts[parts.length - 1];

            DocumentFile destination =
                    currentFolder.findFile(fileName);

            if (destination != null)
                destination.delete();

            destination =
                    currentFolder.createFile(
                            "application/octet-stream",
                            fileName
                    );

            if (destination == null)
                return;

            try (InputStream input =
                         getContentResolver().openInputStream(
                                 source.getUri()
                         );
                 OutputStream output =
                         getContentResolver().openOutputStream(
                                 destination.getUri()
                         ))
            {
                if (input == null || output == null)
                    return;

                byte[] buffer = new byte[8192];
                int count;

                while ((count = input.read(buffer)) != -1)
                {
                    output.write(buffer, 0, count);
                }
            }

            AddMessage(
                    "Backup: " + relativePath
            );
        }
        catch (Exception e)
        {
            AddMessage(
                    "Failed to backup " +
                            filePath +
                            ": " +
                            e.getMessage()
            );
        }
    }

    public boolean createFolder(String parentUri, String folderName)
    {
        try
        {
            Uri uri = Uri.parse(parentUri);

            DocumentFile parent =
                    DocumentFile.fromTreeUri(this, uri);

            if (parent == null || !parent.canWrite())
                return false;

            DocumentFile folder =
                    parent.findFile(folderName);

            if (folder != null)
                return folder.isDirectory();

            folder = parent.createDirectory(folderName);

            return folder != null;
        }
        catch (Exception e)
        {
            e.printStackTrace();
            return false;
        }
    }
}