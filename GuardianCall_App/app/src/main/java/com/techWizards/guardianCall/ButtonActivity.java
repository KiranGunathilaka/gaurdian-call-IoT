package com.techWizards.guardianCall;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class ButtonActivity extends AppCompatActivity {

    private DatabaseReference devicesDatabase;
    private String deviceId;
    private Button backButton;
    private LinearLayout linearLayout;
    private SharedPreferences sharedPreferences , buttonNames;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_button);

        backButton = findViewById(R.id.back_to_alarms);
        linearLayout = findViewById(R.id.linearLayout);

        sharedPreferences = getSharedPreferences("loginDetails", MODE_PRIVATE);
        deviceId = sharedPreferences.getString("deviceId", "defaultStringValue");

        devicesDatabase = FirebaseDatabase.getInstance().getReference("Devices").child(deviceId).child("Buttons");

        devicesDatabase.addValueEventListener(new ValueEventListener() {
            String btnId , batteryPerc , realName;
            Double battery;
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                linearLayout.removeAllViews();

                buttonNames = getSharedPreferences("ButtonNames", MODE_PRIVATE);
                SharedPreferences.Editor editor = buttonNames.edit();

                for (DataSnapshot btn : snapshot.getChildren()){
                    View view = getLayoutInflater().inflate(R.layout.button_card, null);

                    btnId = String.valueOf(btn.getKey());

                    //this will remove if the corrupted button id is fed to the database if the espNow packets are corrupted
                    //this is automatically deleted by the notification service, just in case
                    if(Integer.parseInt(btnId)/100 != Integer.parseInt(deviceId) ){
                        FirebaseDatabase.getInstance().getReference("Devices").child(deviceId).child("Buttons").child(btnId).removeValue();
                        continue;
                    }

                    realName = buttonNames.getString(btnId, btnId ); //if there's no string under btnId in preferences, default string will be btnId too
                    EditText btnName = view.findViewById(R.id.name);
                    btnName.setHint(realName);
                    btnName.setId(Integer.parseInt(btnId));

                    btnName.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                        @Override
                        public void onFocusChange(View v, boolean hasFocus) {
                            String temp = btnName.getText().toString().trim();
                            if (!hasFocus && !temp.isEmpty()) {
                                editor.putString(String.valueOf(btnName.getId()), temp);
                                editor.apply();
                                btnName.setText(null);
                                btnName.setHint(temp);
                                btnName.setNextFocusDownId(R.id.back_to_alarms);
                            }
                        }
                    });

                    btnName.setOnKeyListener(new View.OnKeyListener() {
                        @Override
                        public boolean onKey(View v, int keyCode, KeyEvent event) {
                            if ((event.getAction() == KeyEvent.ACTION_DOWN) && (keyCode == KeyEvent.KEYCODE_ENTER)) {
                                btnName.clearFocus();
                                return true;
                            }
                            return false;
                        }
                    });

                    battery = Double.parseDouble(btn.child("Battery").getValue().toString());
                    batteryPerc = battery< 3.2? 0 + "%" : battery >= 4.2 ? 100+ "%" :Math.floor((battery -3.2) * 100) + "%"; // This is calculated by considering the max and min voltages of the battery as 4.2V and 3.2V
                    TextView strBattery = view.findViewById(R.id.batteryPercentage);
                    strBattery.setText(batteryPerc);

                    LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                    layoutParams.setMargins(0, 10, 0, 10);

                    linearLayout.addView(view, layoutParams);
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });

        backButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
    }


}