package me.aflak.libraries;

import android.graphics.Color;
import android.hardware.usb.UsbDevice;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import io.github.controlwear.virtual.joystick.android.JoystickView;
import me.aflak.arduino.Arduino;
import me.aflak.arduino.ArduinoListener;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.lang.Math;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URI;
import java.net.URISyntaxException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class MainActivity extends AppCompatActivity implements ArduinoListener {
    private Arduino arduino;
    private TextView textView;
    private EditText editText;
    private SocketClient sc = new SocketClient( new URI( "ws://172.19.131.80:9000" ), this);

    public MainActivity() throws URISyntaxException {
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //msgList = findViewById(R.id.msgList);
        //edMessage = findViewById(R.id.edMessage);

        textView = findViewById(R.id.textView);
        textView.setMovementMethod(new ScrollingMovementMethod());
        editText = (EditText) findViewById(R.id.editText);
        arduino = new Arduino(this);
        display("Please plug an Arduino via OTG.\nOn some devices you will have to enable OTG Storage in the phone's settings.\n\n");
        JoystickView joystick = (JoystickView) findViewById(R.id.joystick);

        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                StringBuilder sb = new StringBuilder();
                double x = 1.27 * strength * Math.cos(Math.toRadians(angle));
                double y = 1.27 * strength * Math.sin(Math.toRadians(angle));
                sb.append("move;");
                sb.append((int) x + ";");
                sb.append((int) y + ";");
                sb.append("\r\n");
                arduino.send(sb.toString().getBytes());
                textView.append(x + ", " + y  + "\n" );
            }
        }, 100);
    }
    public void sendCommand(String str) {
        StringBuilder sb = new StringBuilder();
        int x = 0;
        int y = 0;
        if(str.equals("up;"))
        {
            x = 0;
            y = 100;
        }
        if(str.equals("down;"))
        {
            x = 0;
            y = -100;
        }
        if(str.equals("left;"))
        {
            x = -100;
            y = 0;
        }
        if(str.equals("right;"))
        {
            x = 100;
            y = 0;
        }
        if(str.equals("leftup;"))
        {
            x = -50;
            y = 50;
        }
        if(str.equals("rightup;"))
        {
            x = 50;
            y = 50;
        }
        if(str.equals("none;"))
        {
            x = 0;
            y = 0;
        }
        sb.append("move;");
        sb.append(x + ";");
        sb.append(y + ";");
        sb.append("\r\n");
        arduino.send(sb.toString().getBytes());
        display(str + " : Socket sent: " + x + ", " + y);
    }
    public void onClick(View view) {
        sc.connect();
        display("Button clicked");
    }
    @Override
    protected void onStart() {
        super.onStart();
        arduino.setArduinoListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        arduino.unsetArduinoListener();
        arduino.close();
    }

    @Override
    public void onArduinoAttached(UsbDevice device) {
        display("Arduino attached!");
        arduino.open(device);
    }

    @Override
    public void onArduinoDetached() {
        display("Arduino detached");
    }

    @Override
    public void onArduinoMessage(byte[] bytes) {
        display("> "+new String(bytes));
    }

    @Override
    public void onArduinoOpened() {
        String str = "Hello World !";
        arduino.send(str.getBytes());
    }

    @Override
    public void onUsbPermissionDenied() {
        display("Permission denied... New attempt in 3 sec");
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                arduino.reopen();
            }
        }, 3000);
    }

    public void display(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                textView.append(message+"\n");
            }
        });
    }
}
