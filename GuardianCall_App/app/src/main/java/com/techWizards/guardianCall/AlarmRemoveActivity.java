package com.techWizards.guardianCall;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class AlarmRemoveActivity extends AppCompatActivity {
    private TextView backIcon , msgContainer;
    private Button removeBtn;
    private DatabaseReference alarmsDatabase;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_alarm_remove);

        Intent intent = getIntent();
        String description = intent.getStringExtra("msg");
        String time = intent.getStringExtra("time");
        System.out.println(time);
        String[] arr = intent.getStringArrayExtra("daysArr");
        String deviceId = intent.getStringExtra("deviceId");
        String amPm = intent.getStringExtra("amPm");

        if (amPm.equals("pm")){
            time = String.valueOf(1200 + Integer.parseInt(time));
        }

        msgContainer = findViewById(R.id.msgContainer);
        msgContainer.setText(description);

        String path = "Devices/" + deviceId + "/Alarms";
        alarmsDatabase = FirebaseDatabase.getInstance().getReference(path);

        backIcon = findViewById(R.id.backIcon);
        backIcon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        removeBtn = findViewById(R.id.removeButton);
        String finalTime = time;
        removeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final int totalDeletions = countOnes(arr);
                final int[] performedDeletions = {0};
                for (int i =0 ; i<7 ; i++) {
                    if (arr[i].equals("1")) {
                        int finalI = i;
                        System.out.println(finalTime);

                        alarmsDatabase.child(String.valueOf(finalI)).child(finalTime).removeValue()
                                .addOnSuccessListener(new OnSuccessListener<Void>() {
                                    @Override
                                    public void onSuccess(Void aVoid) {
                                        performedDeletions[0]++;
                                        if (totalDeletions == performedDeletions[0]){
                                            Toast.makeText(AlarmRemoveActivity.this, "Alarm deleted", Toast.LENGTH_SHORT).show();
                                            finish();
                                        }
                                    }
                                })
                                .addOnFailureListener(new OnFailureListener() {
                                    @Override
                                    public void onFailure(@NonNull Exception e) {
                                        Toast.makeText(AlarmRemoveActivity.this, "Failed to delete Alarm", Toast.LENGTH_SHORT).show();
                                    }
                                });

                    }
                }
            }
        });
    }

    public static int countOnes(String[] array) {
        int count = 0;
        for (String num : array) {
            if (num.equals("1")) {
                count++;
            }
        }
        return count;
    }
}

