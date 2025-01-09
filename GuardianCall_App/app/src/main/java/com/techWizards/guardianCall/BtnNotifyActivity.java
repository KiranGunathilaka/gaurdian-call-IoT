package com.techWizards.guardianCall;

import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.NoCopySpan;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.SavedStateHandle;

public class BtnNotifyActivity extends AppCompatActivity {

    private Button okBtn;
    private SharedPreferences sharedPreferences;
    private String[] arr;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_btn_notify);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            arr = extras.getStringArray("Buttons");
        }

        sharedPreferences = getSharedPreferences("ButtonNames", MODE_PRIVATE);

        LinearLayout linearLayout = findViewById(R.id.linearLayout);
        linearLayout.removeAllViews();

        int i = 1;
        for (String str: arr){
            View view = getLayoutInflater().inflate(R.layout.btn_name_card, null);

            String temp =  sharedPreferences.getString(str, str);
            TextView txtVw = view.findViewById(R.id.buttonName);
            txtVw.setText(temp);

            LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
            layoutParams.setMargins(0, 7, 0, 7);

            linearLayout.addView(view, layoutParams);
            i++;
        }

        okBtn = findViewById(R.id.okButton);
        okBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (NotificationService.mp != null && NotificationService.mp.isPlaying()) {
                    NotificationService.mp.stop();
                    NotificationService.mp.release();
                    NotificationService.mp = null;
                }


                NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

                Intent intent = getIntent();
                int notificationId = intent.getIntExtra("notificationId", -1);
                System.out.println(notificationId);

                if (notificationId != -1) {
                    notificationManager.cancel(notificationId);
                }

                finish();
            }
        });
    }
}
