package com.techWizards.guardianCall;

public class User {
    private String deviceId;
    private String email;

    public User() {
    }

    public User(String deviceId , String email){
        this.deviceId = deviceId;
        this.email = email;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }
}