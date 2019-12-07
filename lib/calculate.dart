import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'dart:ffi';  // For FFI
import 'dart:io';   // For Platform.isX

final DynamicLibrary nativeAddLib =
Platform.isAndroid
    ? DynamicLibrary.open("libnative_add.so")
    : DynamicLibrary.open("native_add.framework/native_add");

final int Function(int x, int y) nativeAdd =
nativeAddLib
    .lookup<NativeFunction<Int32 Function(Int32, Int32)>>("native_add")
    .asFunction();

final int Function(int x) putChar =
nativeAddLib
    .lookup<NativeFunction<Int32 Function(Int32)>>("putChar")
    .asFunction();

final int Function() getChar =
nativeAddLib
    .lookup<NativeFunction<Int32 Function()>>("getChar")
    .asFunction();


// int argc, char *argv[]

class Calculate {
  static const MethodChannel _channel =
      const MethodChannel('calculate');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}

void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      platformVersion = await Calculate.platformVersion;
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {

    String command = "165513123";
    for(int i=0;i<command.length;i++){
      putChar(command.codeUnitAt(i));
    }

    String msg = "";

    try {
      putChar(0);
    } catch(e){
      msg = e.toString();
    }

    String answer = "x";
    while(true){
      int char = getChar();
      if(char == 0) break;
      answer += String.fromCharCode(char)+",";
    }

    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Text('$msg x$answer'),// / ${nativeExecute()}
        ),
      ),
    );
  }
}
