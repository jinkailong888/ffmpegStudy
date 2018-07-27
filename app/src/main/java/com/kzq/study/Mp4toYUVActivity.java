package com.kzq.study;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class Mp4toYUVActivity extends AppCompatActivity {

    Button convertBtn;

    EditText fileEdt;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mp4to_yuv);

        convertBtn = findViewById(R.id.convertBtn);
        fileEdt = findViewById(R.id.fileEdt);

        convertBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                convertBtn.setEnabled(false);
                new Thread(new Runnable() {
                    @Override
                    public void run() {

                        JniTest.mp4toyuv(fileEdt.getText().toString());
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                convertBtn.setEnabled(true);
                            }
                        });

                    }
                });
            }
        });
    }

}
