package com.techWizards.guardianCall;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Patterns;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.android.material.snackbar.Snackbar;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.Query;
import com.google.firebase.database.ValueEventListener;

public class RegisterActivity extends AppCompatActivity {
    private FirebaseAuth auth;
    private DatabaseReference usersDatabase, devicesDatabase;
    private EditText email , pwd , confirmPwd , devId , secKey;
    private Button registerButton;
    private TextView loginRedirect;

    private boolean usernameExists , passwordsMatch , deviceIdExists , isSecretKeyMatched;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);

        auth = FirebaseAuth.getInstance();
        usersDatabase = FirebaseDatabase.getInstance().getReference("Users");
        devicesDatabase = FirebaseDatabase.getInstance().getReference("Devices");


        email = findViewById(R.id.registerEmail);
        pwd = findViewById(R.id.registerPwd);
        confirmPwd = findViewById(R.id.registerConfPwd);
        devId = findViewById(R.id.registerDeviceID);
        secKey = findViewById(R.id.registerSecretkey);
        loginRedirect = findViewById(R.id.signInRedirectText);
        registerButton = findViewById(R.id.registerButton);

        email.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {

                String userName = email.getText().toString().trim();
                if (!userName.isEmpty()){
                    checkUsernameExists(userName);
                }

            }
        });

        confirmPwd.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                if (!pwd.getText().toString().trim().isEmpty()) {
                    if (pwd.getText().toString().trim().equals(confirmPwd.getText().toString().trim())) {
                        Snackbar.make(confirmPwd, "Password confirmed", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.GREEN).show();
                        passwordsMatch = true;
                    } else {
                        Snackbar.make(confirmPwd, "Passwords do not match", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.RED).show();
                        passwordsMatch = false;
                    }
                }
            }
        });

        devId.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                String temp = devId.getText().toString().trim();
                if (!hasFocus && !temp.isEmpty()) {
                    checkDeviceIdExists(temp);
                }
            }
        });


        registerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String user = email.getText().toString().trim();
                String password = pwd.getText().toString().trim();
                String confPassword = confirmPwd.getText().toString().trim();
                String deviceId = devId.getText().toString().trim();
                String secretKey = secKey.getText().toString().trim();

                if (user.isEmpty()) {
                    email.setError("Email can't be empty");
                } else if (usernameExists) {
                    email.setError("Email already exists");
                } else if (password.isEmpty()) {
                    pwd.setError("Password can't be empty");
                } else if (confPassword.isEmpty() || !confPassword.equals(password)) {
                    confirmPwd.setError("Passwords do not match");
                } else if (deviceId.isEmpty()) {
                    devId.setError("Device ID can't be empty");
                } else if (secretKey.isEmpty()) {
                    secKey.setError("Secret Key can't be empty");
                } else {
                    checkKeyMatchWithDevID(secretKey, deviceId, new KeyMatchCallback() {
                        @Override
                        public void onKeyMatch(boolean isMatched) {
                            if (isMatched) {
                                auth.createUserWithEmailAndPassword(user, password).addOnCompleteListener(new OnCompleteListener<AuthResult>() {
                                    @Override
                                    public void onComplete(@NonNull Task<AuthResult> task) {
                                        if (task.isSuccessful()) {
                                            FirebaseUser firebaseUser = task.getResult().getUser();
                                            String userId = firebaseUser.getUid();

                                            usersDatabase.child(userId).child("email").setValue(user);
                                            usersDatabase.child(userId).child("deviceId").setValue(deviceId);

                                            devicesDatabase.child(deviceId).child("Alarms").addListenerForSingleValueEvent(new ValueEventListener() {
                                                @Override
                                                public void onDataChange(@NonNull DataSnapshot snapshot) {
                                                    if (!snapshot.exists()){
                                                        for(int i=0; i<7; i++){
                                                            devicesDatabase.child(deviceId).child("Alarms").child(String.valueOf(i)).child("0").setValue("");
                                                        }
                                                        String firstBtnID = deviceId+"01";
                                                        devicesDatabase.child(deviceId).child("Buttons").child(firstBtnID).child("Status").setValue(0);
                                                        devicesDatabase.child(deviceId).child("Buttons").child(firstBtnID).child("Battery").setValue(0);

                                                    }
                                                    Toast.makeText(RegisterActivity.this, "Successfully Registered", Toast.LENGTH_SHORT).show();
                                                    startActivity(new Intent(RegisterActivity.this, SignInActivity.class));
                                                    finish();
                                                }

                                                @Override
                                                public void onCancelled(@NonNull DatabaseError error) {
                                                }
                                            });
                                        } else {
                                            String errorMessage = task.getException() != null ? task.getException().getMessage() : "Unknown error";
                                            Toast.makeText(RegisterActivity.this, "Registering Failed, " + errorMessage, Toast.LENGTH_SHORT).show();
                                        }
                                    }
                                });
                            }
                        }
                    });
                }
            }
        });

        loginRedirect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

    }

    private boolean checkUsernameExists(String eMail) {
        Query query = usersDatabase.orderByChild("email").equalTo(eMail);
        if (Patterns.EMAIL_ADDRESS.matcher(eMail).matches()) {
            query.addListenerForSingleValueEvent(new ValueEventListener() {
                @Override
                public void onDataChange(DataSnapshot dataSnapshot) {
                    if (dataSnapshot.exists()) {
                        Snackbar.make(email, "Email already exists", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.RED).show();
                        usernameExists = true;
                    } else {
                        Snackbar.make(email, "Email is available", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.GREEN).show();
                        usernameExists = false;
                    }
                }

                @Override
                public void onCancelled(DatabaseError databaseError) {
                    // Handle possible errors.
                    Toast.makeText(RegisterActivity.this, "Database error: " + databaseError.getMessage(), Toast.LENGTH_SHORT).show();
                }
            });
        }else{
            Snackbar.make(email, "Not a valid Email address", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.RED).show();
        }
        return usernameExists;
    }

    private boolean checkDeviceIdExists(String checkId) {
        devicesDatabase.child(checkId).addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                if (dataSnapshot.exists()) {
                    Snackbar.make(email, "Valid Device ID", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.GREEN).show();
                    deviceIdExists = true;
                } else {
                    Snackbar.make(email, "Device ID does not exist", Snackbar.LENGTH_SHORT).setBackgroundTint(Color.RED).show();
                    devId.setError("Device ID does not exist");
                    deviceIdExists = false;
                }
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {
                // Handle possible errors.
                Toast.makeText(RegisterActivity.this, "Database error: " + databaseError.getMessage(), Toast.LENGTH_SHORT).show();
            }
        });
        return deviceIdExists;
    }

    public interface KeyMatchCallback {
        void onKeyMatch(boolean isMatched);
    }

    private void checkKeyMatchWithDevID(String key, String checkID, KeyMatchCallback callback) {
        devicesDatabase.child(checkID).addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    String databaseKey = snapshot.child("Secret Key").getValue(String.class);
                    boolean isMatched = databaseKey.equals(key);
                    callback.onKeyMatch(isMatched);
                    if (!isMatched) {
                        Toast.makeText(RegisterActivity.this, "Incorrect Device ID or Secret Key", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    callback.onKeyMatch(false);
                    Toast.makeText(RegisterActivity.this, "Incorrect Device ID or Secret Key", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                callback.onKeyMatch(false);
                Toast.makeText(RegisterActivity.this, "Database error: " + error.getMessage(), Toast.LENGTH_SHORT).show();
            }
        });
    }
}