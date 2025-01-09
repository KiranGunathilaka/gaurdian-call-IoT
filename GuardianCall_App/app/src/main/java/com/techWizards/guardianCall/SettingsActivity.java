package com.techWizards.guardianCall;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.material.snackbar.Snackbar;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.Query;
import com.google.firebase.database.ValueEventListener;

import java.util.ArrayList;
import java.util.HashMap;

public class SettingsActivity extends AppCompatActivity {

    private String deviceId, loggedEmail;
    private DatabaseReference usersDatabase;
    private Button logoutBtn , removeBtn;
    private TextView backIcon, devIdField;
    private SharedPreferences sharedPreferences;
    private LinearLayout linearLayout;
    private HashMap<TextView , Boolean> textViewState = new HashMap<>();
    private ArrayList<String> removingEmails = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        sharedPreferences = getSharedPreferences("loginDetails", MODE_PRIVATE);
        deviceId = sharedPreferences.getString("deviceId", "defaultStringValue");
        loggedEmail = sharedPreferences.getString("userEmail", "defaultStringValue");

        backIcon = findViewById(R.id.backIcon);
        logoutBtn = findViewById(R.id.logOutButton);
        devIdField = findViewById(R.id.deviceId);
        removeBtn = findViewById(R.id.removeButton);

        linearLayout = findViewById(R.id.linearLayout);

        //Setting devID and current User email in the relevant TextViews
        devIdField.setText(deviceId);

        usersDatabase = FirebaseDatabase.getInstance().getReference("Users");
        Query query = usersDatabase.orderByChild("deviceId").equalTo(deviceId);
        query.addValueEventListener(new ValueEventListener() {

            String email;
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                linearLayout.removeAllViews();

                // This is to add the current user email at the top and bigger
                View view0 = getLayoutInflater().inflate(R.layout.email_card, null);
                TextView textView0 = view0.findViewById(R.id.emailsTextView);
                textView0.setText(loggedEmail);
                textView0.setPadding(30,15,0,15);
                textView0.setTextSize(21);

                LinearLayout.LayoutParams layoutParams0 = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);

                layoutParams0.setMargins(0, 5, 0, 5);

                linearLayout.addView(view0 , layoutParams0);

                for (DataSnapshot snap : snapshot.getChildren()) {
                    //this is why defined another user class. See DataSnapShot documentation
                    User user = snap.getValue(User.class);
                    email = user.getEmail().toString();

                    if (!email.equals(loggedEmail)){
                        //assigning the custom made textView component to view object to add it to the linearLayout container
                        View view = getLayoutInflater().inflate(R.layout.email_card, null);
                        TextView textView = view.findViewById(R.id.emailsTextView);
                        textView.setText(email);
                        textView.setPadding(30,16,0,16);
                        textViewState.put(textView, false);

                        textView.setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                boolean currentState = textViewState.get(textView);
                                String rmEmail = textView.getText().toString().trim();
                                if (!loggedEmail.equals(rmEmail)){
                                    if (!currentState){
                                        removingEmails.add(rmEmail);
                                        textViewState.put(textView , true);
                                        textView.setBackgroundResource(R.drawable.text_box_order_red);
                                    }else{
                                        removingEmails.remove(textView.getText().toString().trim());
                                        textViewState.put(textView, false);
                                        textView.setBackgroundResource(R.drawable.text_box_border);
                                    }
                                } else{
                                    Snackbar.make(textView, "Can't remove currently logged in user", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.RED).show();
                                }
                            }
                        });

                        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);

                        layoutParams.setMargins(0, 5, 0, 5);

                        linearLayout.addView(view , layoutParams);
                    }

                }
            }

            public void onCancelled(DatabaseError databaseError) {
                // Handle possible errors
                Toast.makeText(SettingsActivity.this, "User loading failed : " + databaseError, Toast.LENGTH_SHORT).show();
                Log.w("FirebaseQuery", "loadPost:onCancelled", databaseError.toException());
            }
        });

        backIcon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        logoutBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SharedPreferences sharedPreferences = getSharedPreferences("loginDetails", MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPreferences.edit();
                editor.clear();
                editor.apply();

                stopService(new Intent(SettingsActivity.this , NotificationService.class));

                Intent intent = new Intent(SettingsActivity.this, SignInActivity.class);
                // Setting flags to clear the current task and start a new task with SignInActivity as the root
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                startActivity(intent);
                finish();
            }
        });

        removeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showConfirmationDialog();
            }
        });

    }



    private void showConfirmationDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(SettingsActivity.this);
        builder.setTitle("Confirmation");
        builder.setMessage("Are you sure you want to delete these user/users?\nThey won't be able to register again with the same email!!");


        builder.setPositiveButton("Remove", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                for (String str : removingEmails){
                    Query query2 = usersDatabase.orderByChild("email").equalTo(str);
                    query2.addListenerForSingleValueEvent(new ValueEventListener() {
                        @Override
                        public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                            for (DataSnapshot childSnapshot : dataSnapshot.getChildren()) {
                                String key = childSnapshot.getKey();
                                    usersDatabase.child(key).removeValue()
                                            .addOnSuccessListener(new OnSuccessListener<Void>() {
                                                @Override
                                                public void onSuccess(Void unused) {
                                                    Toast.makeText(SettingsActivity.this , "User removed", Toast.LENGTH_SHORT).show();
                                                }
                                            })
                                            .addOnFailureListener(new OnFailureListener() {
                                                @Override
                                                public void onFailure(@NonNull Exception e) {
                                                    Toast.makeText(SettingsActivity.this , "Action failed" , Toast.LENGTH_SHORT).show();
                                                }
                                            });

                            }
                        }

                        @Override
                        public void onCancelled(@NonNull DatabaseError databaseError) {
                            // Handle the error
                        }
                    });
                }

            }
        });

        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                removingEmails.clear();
            }
        });

        AlertDialog dialog = builder.create();
        dialog.show();
    }

}