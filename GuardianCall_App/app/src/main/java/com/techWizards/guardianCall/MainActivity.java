package com.techWizards.guardianCall;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.cardview.widget.CardView;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {

    private String deviceId;
    private DatabaseReference alarmsDatabase;
    private Button buttonsRedirect;
    private TextView settings , setAlarm ;
    private LinearLayout linearLayout;
    private SharedPreferences sharedPreferences;
    private String[] weekdaysArr = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    private String[][][] alarmsArr = new String[7][][];
    private ArrayList<String[][]> structuredArr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_alarms);


        settings = findViewById(R.id.settingsIcon);
        setAlarm = findViewById(R.id.setNewAlarmIcon);
        buttonsRedirect = findViewById(R.id.to_buttons);

        linearLayout = findViewById(R.id.linearLayout);

        sharedPreferences = getSharedPreferences("loginDetails", MODE_PRIVATE);
        deviceId = sharedPreferences.getString("deviceId", "defaultStringValue");

        startService(new Intent(MainActivity.this, NotificationService.class));

        alarmsDatabase = FirebaseDatabase.getInstance().getReference("Devices").child(deviceId).child("Alarms");
        alarmsDatabase.addValueEventListener(new ValueEventListener() {
            //handle null pointer exception if week day is empty later
            //getting alarm data from the firebase and assigning them to array[weekday][nth alarm][alarmTime, message]
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                linearLayout.removeAllViews();
                int j =0;
                for (DataSnapshot daySnapshot : snapshot.getChildren()) {
                    String[][] dayAlarmsList = new String[(int) daySnapshot.getChildrenCount()][];
                    int i =0;
                    for(DataSnapshot childSnapShot : daySnapshot.getChildren()){
                        String time = childSnapShot.getKey().toString();
                        String message = childSnapShot.getValue().toString();

                        String[] singleAlarm = {time , message};

                        dayAlarmsList[i] = singleAlarm;
                        i++;
                    }
                    alarmsArr[j] = dayAlarmsList;
                    j++;
                }

                structuredArr = restructureArray(alarmsArr);

                int time , hh , mm;
                String amPm , description , daysStr , timeStr;

                for (String[][] element : structuredArr){
                    description = element[1][0];
                    time = Integer.parseInt(element[0][0]);
                    hh = (int) Math.floor((double) time/100);
                    mm = time % 100 ;

                    if (hh > 12){
                        hh = hh -12;
                        amPm = "pm";
                    }else{
                        amPm = "am";
                    }


                    String HH , MM;
                    if (hh < 10){
                        HH = "0" + String.valueOf(hh);
                    }else{
                        HH = String.valueOf(hh);
                    }

                    if (mm < 10){
                        MM  = "0" + String.valueOf(mm);
                    }else{
                        MM  = String.valueOf(mm);
                    }

                    timeStr = HH + ":" + MM;

                    daysStr = "";
                    int i =0;
                    for (String d : element[2]){
                        if (d.equals("1")){
                            daysStr += weekdaysArr[i]+" ";
                        }
                        if (daysStr.equals("Sun Mon Tue Wed Thu Fri Sat ")){
                            daysStr = "Everyday";
                        }
                        i++;
                    }

                    View view = getLayoutInflater().inflate(R.layout.alarm_card, null);

                    TextView titleTextView = view.findViewById(R.id.titleTextView);
                    titleTextView.setText(description);

                    TextView timeTextView = view.findViewById(R.id.timeTextView);
                    timeTextView.setText(timeStr);

                    TextView daysTextView = view.findViewById(R.id.daysTextView);
                    daysTextView.setText(daysStr);

                    TextView ampmTextView = view.findViewById(R.id.ampmTextView);
                    ampmTextView.setText(amPm);

                    CardView cardView = view.findViewById(R.id.cardView);

                    String finalDescription = description;
                    String finalTime = String.valueOf(hh + MM);
                    String finalAmPm = amPm;
                    cardView.setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Intent intent = new Intent(MainActivity.this , AlarmRemoveActivity.class);
                            intent.putExtra("msg", finalDescription);
                            intent.putExtra("time", finalTime);
                            intent.putExtra("amPm", finalAmPm);
                            intent.putExtra("daysArr", element[2]);
                            intent.putExtra("deviceId" , deviceId);
                            startActivity(intent);
                        }
                    });

                    LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);

                    layoutParams.setMargins(0, 8, 0, 8);

                    linearLayout.addView(view, layoutParams);
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                Toast.makeText(MainActivity.this , "Database Error : " + error , Toast.LENGTH_SHORT).show();
            }
        });

        settings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, SettingsActivity.class);
                startActivity(intent);
            }
        });

        buttonsRedirect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, ButtonActivity.class);
                startActivity(intent);
            }
        });

        setAlarm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, SetAlarmActivity.class);
                startActivity(intent);
            }
        });
    }

    private ArrayList<String[][]> restructureArray(String[][][] arr){
        ArrayList<String[][]> list = new ArrayList<>();
        String tempTime , tempDes;
        for (int i =0 ; i < 7 ; i++){

            for (int j =0 ; j < arr[i].length ; j++){
                tempTime = arr[i][j][0];
                tempDes = arr[i][j][1].trim();

                String[] dayRecorder= {"0", "0", "0", "0","0", "0", "0"};
                dayRecorder[i] = "1";

                if (tempTime != "0" && !tempDes.equals("")){
                    // int l = j; when setting alarms, don't let user to set two alarms at same day, same time
                    //even though app doesn't get affected, IoT device may have errors
                    int l = 0;
                    //use k = i+1 also as now we don't have to find matching alarms in the same day

                    for (int k =i+1  ; k< 7-i ;k++) {

                        while ( l < arr[k].length){
                            if(tempTime.intern() == arr[k][l][0].intern()){
                                //There's a bug that prevent below code from passing if condition even though  strings are equal . Usually it happens if the equal key value pair nodes in the firebase are created simultaneously.
                                if (tempDes.equals(arr[k][l][1].trim()) ){
                                    dayRecorder[k] = "1";
                                    arr[k][l][0] = "0";
                                }
                            }
                            l++;
                        }
                        l=0;
                    }
                    String[][] newAlarm = new String[][] {{tempTime}, {tempDes}, dayRecorder};
                    list.add(newAlarm);
                }
            }
        }

        return list;
    }

}
