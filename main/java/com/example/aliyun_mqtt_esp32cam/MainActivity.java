package com.example.aliyun_mqtt_esp32cam;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/****************mqtt_init函数里，mqtt的五要素需要更改*********************/
/****************发送json数据的格式也要注意*********************/
public class MainActivity extends AppCompatActivity {

    private MqttClient client;
    private MqttConnectOptions options;
    private Handler handler;
    private ScheduledExecutorService scheduler;
    private String productKey="k28otXwYWjm";
    private String deviceName="app-dev";

    private String deviceSecret="b967f331dddf5fde78c61a1227333711";
    private  int temperature=0;
    private  int humidity=0;

    private final String pub_topic="/sys/k28otXwYWjm/app-dev/thing/event/property/post";

    private final String sub_topic="/sys/k28otXwYWjm/app-dev/thing/service/property/set";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tv_temp = findViewById(R.id.tv_temp);
        TextView tv_humi = findViewById(R.id.tv_humi);

        Button btn_open = findViewById(R.id.btn_open);
        Button btn_close = findViewById(R.id.btn_close);

        btn_open.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                /*开*/
                publish_message("{\"params\":{\"led\":1},\"version\":\"1.0.0\"}");
            }
        });

        btn_close.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                /*关*/
                publish_message("{\"params\":{\"led\":0},\"version\":\"1.0.0\"}");
            }
        });

        mqtt_init();
        start_reconnect();

        handler = new Handler() {
            @SuppressLint("SetTextI18n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 1: //开机校验更新回传
                        break;
                    case 2:  // 反馈回传
                        break;
                    case 3:  //MQTT 收到消息回传   UTF8Buffer msg=new UTF8Buffer(object.toString());
                        String message = msg.obj.toString();
                        Log.d("nicecode", "handleMessage: "+ message);
                        try {
                            JSONObject jsonObjectALL = null;
                            jsonObjectALL = new JSONObject(message);
                            JSONObject items = jsonObjectALL.getJSONObject("items");

                            JSONObject obj_temp = items.getJSONObject("temp");
                            JSONObject obj_humi = items.getJSONObject("humi");

                            temperature = obj_temp.getInt("value");
                            humidity = obj_humi.getInt("value");

                            tv_temp.setText(temperature + "");
                            tv_humi.setText(humidity + "");
                            Log.d("nicecode", "temp: "+ temperature);
                            Log.d("nicecode", "humi: "+ humidity);
                        } catch (JSONException e) {
                            e.printStackTrace();
                            break;
                        }
                        break;
                    case 30:  //连接失败
                        Toast.makeText(MainActivity.this, "连接失败", Toast.LENGTH_SHORT).show();
                        break;
                    case 31:   //连接成功
                        Toast.makeText(MainActivity.this, "连接成功", Toast.LENGTH_SHORT).show();
                        try {
                            client.subscribe(sub_topic, 1);
                        } catch (MqttException e) {
                            e.printStackTrace();
                        }
                        break;
                    default:
                        break;
                }
            }
        };

    }

    private void mqtt_init() {
        try {

            String clientId = "k28otXwYWjm.app-dev";
            Map<String, String> params = new HashMap<String, String>(16);
            params.put("productKey", productKey);
            params.put("deviceName", deviceName);
            params.put("clientId", clientId);
            String timestamp = String.valueOf(System.currentTimeMillis());
            params.put("timestamp", timestamp);
            // cn-shanghai
            String host_url ="tcp://"+ productKey + ".iot-as-mqtt.cn-shanghai.aliyuncs.com:1883";
//            String client_id = clientId + "|securemode=2,signmethod=hmacsha1,timestamp=" + timestamp + "|";
            String user_name = deviceName + "&" + productKey;
//            String password = com.example.aliyun_mqtt_esp32cam.aliyun_mqtt_esp32cam.sign(params, deviceSecret, "hmacsha1");
//            String host_url ="iot-06z00f6xb8psxgr.mqtt.iothub.aliyuncs.com:1883";
            String client_id ="k28otXwYWjm.app-dev|securemode=2,signmethod=hmacsha256,timestamp=1736761343870|";
//            String user_name ="app-dev&k28otXwYWjm";
            String password ="277d2ebab991d27eb202216cf10f44f1ff7369477fad7eb93db114d8e2df57ed";

                    //host为主机名，test为clientid即连接MQTT的客户端ID，一般以客户端唯一标识符表示，MemoryPersistence设置clientid的保存形式，默认为以内存保存
            System.out.println(">>>" + host_url);
            System.out.println(">>>" + client_id);

            //connectMqtt(targetServer, mqttclientId, mqttUsername, mqttPassword);

            client = new MqttClient(host_url, client_id, new MemoryPersistence());
            //MQTT的连接设置
            options = new MqttConnectOptions();
            //设置是否清空session,这里如果设置为false表示服务器会保留客户端的连接记录，这里设置为true表示每次连接到服务器都以新的身份连接
            options.setCleanSession(false);
            //设置连接的用户名
            options.setUserName(user_name);
            //设置连接的密码
            options.setPassword(password.toCharArray());
            // 设置超时时间 单位为秒
            options.setConnectionTimeout(10);
            // 设置会话心跳时间 单位为秒 服务器会每隔1.5*20秒的时间向客户端发送个消息判断客户端是否在线，但这个方法并没有重连的机制
            options.setKeepAliveInterval(60);
            //设置回调
            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    //连接丢失后，一般在这里面进行重连
                    System.out.println("connectionLost----------");
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    //publish后会执行到这里
                    System.out.println("deliveryComplete---------" + token.isComplete());
                }

                @Override
                public void messageArrived(String topicName, MqttMessage message)
                        throws Exception {
                    //subscribe后得到的消息会执行到这里面
                    System.out.println("messageArrived----------");
                    Message msg = new Message();
                    //封装message包
                    msg.what = 3;   //收到消息标志位
                    msg.obj =message.toString();
                    //发送messge到handler
                    handler.sendMessage(msg);    // hander 回传
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void mqtt_connect() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if (!(client.isConnected()))  //如果还未连接
                    {
                        client.connect(options);
                        Message msg = new Message();
                        msg.what = 31;
                        // 没有用到obj字段
                        handler.sendMessage(msg);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    Message msg = new Message();
                    msg.what = 30;
                    // 没有用到obj字段
                    handler.sendMessage(msg);
                }
            }
        }).start();
    }


    private void start_reconnect() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                if (!client.isConnected()) {
                    mqtt_connect();
                }
            }
        }, 0 * 1000, 10 * 1000, TimeUnit.MILLISECONDS);
    }

    private void publish_message(String message) {
        if (client == null || !client.isConnected()) {
            return;
        }
        MqttMessage mqtt_message = new MqttMessage();
        mqtt_message.setPayload(message.getBytes());
        try {
            client.publish(pub_topic, mqtt_message);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }



}
