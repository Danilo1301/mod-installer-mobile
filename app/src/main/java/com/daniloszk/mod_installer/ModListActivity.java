package com.daniloszk.mod_installer;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

public class ModListActivity extends AppCompatActivity
{
    private static final String DATABASE_URL = "https://raw.githubusercontent.com/Danilo1301/mod-installer-mobile/refs/heads/main/README.md";

    private String _installUrl;

    private static final int PICK_GAME_FOLDER = 1001;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mod_list);

        loadRepositories();
    }

    private void loadRepositories()
    {
        new Thread(() ->
        {
            try
            {
                URL url = new URL(DATABASE_URL);

                HttpURLConnection connection =
                        (HttpURLConnection) url.openConnection();

                connection.setRequestMethod("GET");
                connection.setConnectTimeout(10000);
                connection.setReadTimeout(10000);

                int responseCode = connection.getResponseCode();

                if (responseCode != HttpURLConnection.HTTP_OK)
                {
                    throw new IOException(
                            "HTTP " + responseCode
                    );
                }

                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(
                                connection.getInputStream()
                        )
                );

                StringBuilder content = new StringBuilder();

                String line;

                while ((line = reader.readLine()) != null)
                {
                    content.append(line).append('\n');
                }

                reader.close();
                connection.disconnect();

                String readme = content.toString();

                System.out.println(readme);

                runOnUiThread(() ->
                {
                    onRepositoriesLoaded(readme);
                });
            }
            catch (Exception e)
            {
                runOnUiThread(() ->
                {
                    e.printStackTrace();
                });
            }
        }).start();
    }

    private void onRepositoriesLoaded(String readme)
    {
        List<ModRepository> repositories =
                parseRepositories(readme);

        for (ModRepository repository : repositories)
        {
            if (repository.githubUrl != null)
            {
                loadGitHubReleases(repository);
            }

            if (repository.zipUrl != null)
            {
                ModVersion version = new ModVersion();
                version.version = "Download";
                version.downloadUrl = repository.zipUrl;

                repository.versions.add(version);

                onGitHubReleasesLoaded(repository);
            }
        }
    }

    private List<ModRepository> parseRepositories(String readme)
    {
        List<ModRepository> repositories = new ArrayList<>();

        String[] blocks = readme.split("#");

        for (String block : blocks)
        {
            block = block.trim();

            if (block.isEmpty())
                continue;

            ModRepository repository = new ModRepository();

            String[] lines = block.split("\\r?\\n");

            for (String line : lines)
            {
                line = line.trim();

                if (line.startsWith("name = "))
                {
                    repository.name = line.substring(7).trim();
                }
                else if (line.startsWith("description = "))
                {
                    repository.description = line.substring(14).trim();
                }
                else if (line.startsWith("github = "))
                {
                    repository.githubUrl = line.substring(9).trim();
                }
                else if (line.startsWith("zip = "))
                {
                    repository.zipUrl = line.substring(6).trim();
                }
            }

            if (repository.name != null &&
                    !repository.name.isEmpty())
            {
                repositories.add(repository);
            }
        }

        return repositories;
    }

    private void loadGitHubReleases(ModRepository repository)
    {
        new Thread(() ->
        {
            try
            {
                URI uri = URI.create(repository.githubUrl);

                String path = uri.getPath();

                if (path.startsWith("/"))
                    path = path.substring(1);

                String apiUrl =
                        "https://api.github.com/repos/" +
                                path +
                                "/releases";

                URL url = new URL(apiUrl);

                HttpURLConnection connection =
                        (HttpURLConnection) url.openConnection();

                connection.setRequestMethod("GET");
                connection.setConnectTimeout(10000);
                connection.setReadTimeout(10000);
                connection.setRequestProperty(
                        "Accept",
                        "application/vnd.github+json"
                );

                int responseCode = connection.getResponseCode();

                if (responseCode != HttpURLConnection.HTTP_OK)
                {
                    throw new IOException(
                            "HTTP " + responseCode
                    );
                }

                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(
                                connection.getInputStream()
                        )
                );

                StringBuilder content = new StringBuilder();

                String line;

                while ((line = reader.readLine()) != null)
                {
                    content.append(line);
                }

                reader.close();
                connection.disconnect();

                JSONArray releases =
                        new JSONArray(content.toString());

                for (int i = 0; i < releases.length(); i++)
                {
                    JSONObject release =
                            releases.getJSONObject(i);

                    String version =
                            release.getString("tag_name");

                    JSONArray assets =
                            release.getJSONArray("assets");

                    for (int j = 0; j < assets.length(); j++)
                    {
                        JSONObject asset =
                                assets.getJSONObject(j);

                        String downloadUrl =
                                asset.getString(
                                        "browser_download_url"
                                );

                        ModVersion modVersion =
                                new ModVersion();

                        modVersion.version = version;
                        modVersion.downloadUrl = downloadUrl;

                        repository.versions.add(
                                modVersion
                        );
                    }
                }

                runOnUiThread(() ->
                {
                    onGitHubReleasesLoaded(repository);
                });
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }).start();
    }

    private void onGitHubReleasesLoaded(ModRepository repository)
    {
        LinearLayout modList = findViewById(R.id.modList);

        TextView name = new TextView(this);
        name.setText(repository.name);
        name.setTextSize(20);
        name.setPadding(0, 16, 0, 4);

        modList.addView(name);

        TextView description = new TextView(this);
        description.setText(repository.description);
        description.setPadding(0, 0, 0, 12);

        modList.addView(description);

        for (ModVersion version : repository.versions)
        {
            Button button = new Button(this);

            button.setText("Instalar " + version.version);

            button.setOnClickListener(v ->
            {
                _installUrl = version.downloadUrl;

                Intent intent =
                        new Intent(
                                Intent.ACTION_OPEN_DOCUMENT_TREE
                        );

                intent.addFlags(
                        Intent.FLAG_GRANT_READ_URI_PERMISSION |
                                Intent.FLAG_GRANT_WRITE_URI_PERMISSION |
                                Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
                );

                startActivityForResult(
                        intent,
                        PICK_GAME_FOLDER
                );
            });

            modList.addView(button);
        }
    }

    @Override
    protected void onActivityResult(
            int requestCode,
            int resultCode,
            Intent data)
    {
        super.onActivityResult(
                requestCode,
                resultCode,
                data
        );

        if (requestCode != PICK_GAME_FOLDER)
            return;

        if (resultCode != RESULT_OK || data == null)
            return;

        Uri folderUri = data.getData();

        getContentResolver().takePersistableUriPermission(
                folderUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION |
                        Intent.FLAG_GRANT_WRITE_URI_PERMISSION
        );

        Intent intent =
                new Intent(
                        this,
                        ConsoleActivity.class
                );

        intent.putExtra(
                "url",
                _installUrl
        );

        intent.putExtra(
                "game_folder_uri",
                folderUri.toString()
        );

        startActivity(intent);
    }
}