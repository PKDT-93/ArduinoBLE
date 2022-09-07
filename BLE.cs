using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ArduinoBluetoothAPI;
using UnityEngine.Android;
using System.Linq;
using System;
using System.Threading;

public class BLE : MonoBehaviour
{
    private BluetoothHelper helper;
    private static string serviceUUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
    private static string characteristicUUID = "19B10001-E8F2-537E-4F6C-D104768A1214";
    private BluetoothHelperCharacteristic bluetoothHelperCharacteristic;
    public int threshold = 115;
    public float timer;
    public int val;
    private int[] mprArray;
    private byte[] data;

    private byte[] Data
    {
        get
        {
            return data;
        }
        set
        {
            data = value;
        }
    }

    public int [] MPRArray
    {
        get
        {
            return mprArray;
        }
        set
        {
            mprArray = value;
        }
    }

    void Start()
    {
        BluetoothHelper.BLE = true;
        timer = 0;
        val = 0;
        helper = BluetoothHelper.GetInstance("Arduino");
        helper.OnScanEnded += OnScanEnded;
        helper.OnConnected += OnConnected;
        helper.OnConnectionFailed += OnConnectionFailed;
        helper.OnCharacteristicChanged += OnCharacteristicChanged;
        helper.OnCharacteristicNotFound += OnCharacteristicNotFound;
        helper.OnServiceNotFound += OnServiceNotFound;
        helper.ScanNearbyDevices();

        Permission.RequestUserPermission(Permission.CoarseLocation);
        bluetoothHelperCharacteristic = new BluetoothHelperCharacteristic(characteristicUUID, serviceUUID);
    }

    void Update()
    {
        if (!helper.isConnected())
            return;
        ReadArduino();
        Write(1);
        if (Data != null)
        {
            Write(1);
            setData(System.Text.Encoding.ASCII.GetString(Data));
        }
        //for (int i = 0; i < MPRArray.Length; i++)
        //{
        //    if (MPRArray[i] < threshold)
        //    {
        //        Debug.Log("threshold met");
        //        Write(1);
        //    }
        //}
    }

    void OnScanEnded(BluetoothHelper helper, LinkedList<BluetoothDevice> devices)
    {
        Debug.Log("Scan ended");
        if (helper.isDevicePaired())
        {
            helper.Connect();
            Debug.Log(" connecting");
        }
        else
            helper.ScanNearbyDevices();

        foreach (BluetoothDevice d in devices)
            Debug.Log(d.DeviceName);
    }
    void OnConnected(BluetoothHelper helper)
    {
        List<BluetoothHelperService> services = helper.getGattServices();
        foreach (BluetoothHelperService s in services)
        {
            Debug.Log($"Service : [{s.getName()}]");
            foreach (BluetoothHelperCharacteristic c in s.getCharacteristics())
            {
                Debug.Log($"Characteristic : [{c.getName()}]");
            }
        }
        helper.Subscribe(bluetoothHelperCharacteristic);
    }

    void OnConnectionFailed(BluetoothHelper helper)
    {
        Debug.Log("Connection failed");
        helper.ScanNearbyDevices();
    }

    void OnCharacteristicChanged(BluetoothHelper helper, byte[] data, BluetoothHelperCharacteristic characteristic)
    {
        //Debug.Log($"Update value for characteristic [{characteristic.getName()}] of service [{characteristic.getService()}]");
        Debug.Log($"Arduino: [{System.Text.Encoding.ASCII.GetString(data)}]");
        Data = data;
    }

    void OnServiceNotFound(BluetoothHelper helper, string service)
    {
        Debug.Log($"Service [{service}] not found");
    }

    void OnCharacteristicNotFound(BluetoothHelper helper, string service, string characteristic)
    {
        Debug.Log($"Characteristic [{service}] of service [{service}] not found");
    }

    public void Write(string data)
    {
        helper.WriteCharacteristic(bluetoothHelperCharacteristic, data);
    }

    public void Write(int data)
    {
        Debug.Log("Sending data " + data);
        helper.WriteCharacteristic(bluetoothHelperCharacteristic, new byte[] { (byte)data });
    }

    void ReadArduino()
    {
        helper.ReadCharacteristic(bluetoothHelperCharacteristic);
    }

    void setData(string data)
    {
        data = data.Trim();
        MPRArray = Array.ConvertAll(data.Split('\t'), int.Parse);
            for (int i = 0; i < MPRArray.Length; i++)
            {
                if (MPRArray[i] < threshold)
                {
                    Debug.Log("threshold met");
                    Write(1);
                }
            }
        }

    //bool CheckTouch(int[] MPRArray)
    //{
    //    if (MPRArray == null)
    //    {
    //        return false;
    //    }
    //    this.MPRArray = MPRArray;
    //    int threshold = 115;
    //    for (int i = 0; i < MPRArray.Length; i++)
    //    {
    //        if (MPRArray[i] < threshold)
    //        {
    //            Debug.Log("threshold met");
    //            return true;
    //        }
    //    }
    //    return false;
    //}

    void OnDestroy()
    {
        helper.OnScanEnded -= OnScanEnded;
        helper.OnConnected -= OnConnected;
        helper.OnConnectionFailed -= OnConnectionFailed;
        helper.OnCharacteristicChanged -= OnCharacteristicChanged;
        helper.OnCharacteristicNotFound -= OnCharacteristicNotFound;
        helper.OnServiceNotFound -= OnServiceNotFound;
        helper.Disconnect();
    }
}
