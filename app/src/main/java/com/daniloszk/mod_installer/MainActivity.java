package com.daniloszk.mod_installer;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.daniloszk.mod_installer.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("mod_installer");
    }

    protected ActivityMainBinding binding;

    private EditText _modUrl;

    private static final int PICK_GAME_FOLDER = 100;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        _modUrl = binding.modUrl;

        Button installButton = binding.installButton;

        installButton.setOnClickListener(v ->
        {
            String url = _modUrl.getText().toString().trim();

            if (url.isEmpty())
                return;

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

        Button searchButton = binding.searchButton;

        searchButton.setOnClickListener(v ->
        {
            Intent intent = new Intent(
                    MainActivity.this,
                    ModListActivity.class
            );

            startActivity(intent);
        });

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == PICK_GAME_FOLDER)
        {
            Uri folderUri = data.getData();

            getContentResolver().takePersistableUriPermission(folderUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

            String url = _modUrl.getText().toString().trim();

            if(url.isEmpty())
            {
                //url = "https://github.com/Danilo1301/police-mod-szk-auto-install/releases/download/0.2/Mod_Policia.zip";
                return;
            }

            Intent intent = new Intent(this, ConsoleActivity.class);

            intent.putExtra("url", url);
            intent.putExtra("game_folder_uri", folderUri.toString());

            startActivity(intent);
        }
    }

    public native String stringFromJNI();
}