package com.techWizards.guardianCall;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Patterns;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class SignInActivity extends AppCompatActivity {

    private FirebaseAuth auth;
    private EditText signInEmail, signInPwd;
    private Button signInButton;
    private TextView registerRedirect;
    private String deviceId;
    private SharedPreferences loginDetails;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_signin);

        auth =FirebaseAuth.getInstance();
        signInEmail = findViewById(R.id.emailsTextView);
        signInPwd = findViewById(R.id.signInPwd);
        signInButton = findViewById(R.id.logInButton);
        registerRedirect = findViewById(R.id.signInRegRedirect);

        signInButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String email = signInEmail.getText().toString().trim();
                String pass = signInPwd.getText().toString().trim();

                if (!email.isEmpty() && Patterns.EMAIL_ADDRESS.matcher(email).matches()) {
                    if (!pass.isEmpty()) {
                        auth.signInWithEmailAndPassword(email, pass).addOnSuccessListener(new OnSuccessListener<AuthResult>() {
                                    @Override
                                    public void onSuccess(AuthResult authResult) {
                                        Toast.makeText(SignInActivity.this, "Login Successful", Toast.LENGTH_SHORT).show();

                                        String userId = authResult.getUser().getUid();
                                        DatabaseReference usersDatabase = FirebaseDatabase.getInstance().getReference("Users");
                                        usersDatabase.child(userId).addListenerForSingleValueEvent(new ValueEventListener() {
                                            @Override
                                            public void onDataChange(DataSnapshot snapshot) {
                                                if(snapshot.exists()){
                                                    deviceId = snapshot.child("deviceId").getValue(String.class);

                                                    loginDetails = getSharedPreferences("loginDetails", MODE_PRIVATE);
                                                    SharedPreferences.Editor editor = loginDetails.edit();
                                                    editor.putString("userId", userId);
                                                    editor.putString("userEmail", email);
                                                    editor.putString("deviceId", deviceId);
                                                    editor.apply();

                                                    Intent intent = new Intent(SignInActivity.this, MainActivity.class);
                                                    startActivity(intent);
                                                    finish();
                                            } else{
                                                    Toast.makeText(SignInActivity.this, "Something went wrong. Contact customer Service" , Toast.LENGTH_SHORT).show();
                                                }
                                            }

                                            @Override
                                            public void onCancelled(@NonNull DatabaseError error) {
                                                Toast.makeText(SignInActivity.this, "Database error: " + error.getMessage(), Toast.LENGTH_SHORT).show();
                                            }
                                        });


                                    }
                                }).addOnFailureListener(new OnFailureListener() {
                                    @Override
                                    public void onFailure(@NonNull Exception e) {
                                        Toast.makeText(SignInActivity.this, "Login Failed. Check your credentials", Toast.LENGTH_SHORT).show();
                                    }
                                });
                    } else {
                        signInPwd.setError("Password can't be empty");
                    }
                } else if (email.isEmpty()) {
                    signInEmail.setError("Email can't be empty");
                } else {
                    signInEmail.setError("Invalid email address");
                }
            }
        });

        registerRedirect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(SignInActivity.this, RegisterActivity.class);
                startActivity(intent);
            }
        });
    }

}
