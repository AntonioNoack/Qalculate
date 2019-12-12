import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:calculate/calculate.dart';

void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

String compute(String request){

  if(request.isEmpty) return "...";

  for(int i=0;i<request.length;i++){
    putChar(request.codeUnitAt(i));
  }

  try {
    putChar(0);
  } catch(e){
    return e.toString();
  }

  String answer = "";
  while(true){
    int char = getChar();
    if(char == 0) break;
    answer += String.fromCharCode(char);
  }

  return answer;

}

class _MyAppState extends State<MyApp> {

  String _platformVersion = 'Unknown';
  String answer = "...";

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

    // todo main page with different options,
    // todo plotting, formulas, help, ...

    TextEditingController textInputController = TextEditingController();
    TextField textInput = TextField(
      keyboardType: TextInputType.multiline,
      maxLines: null,
      decoration: InputDecoration(
        border: UnderlineInputBorder(),
        hintText: 'Watcha qalculate?',
      ),
      controller: textInputController,
      /*onChanged: (text){
        setState(() {
          answer = compute(text);
        });
      },*/
    );

    Center main = Center(
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 16.0),
          child: Column(
            children: <Widget>[
              Row(
                children: <Widget>[
                  Expanded(
                    child: textInput,
                  ),
                  SizedBox(
                    width: 50,
                    child: RaisedButton(
                      child: Text("Calc!"),
                      onPressed: (){
                        answer = compute(textInputController.text);
                      },
                    ),
                  )
                ],
              ),
              Container(
                padding: const EdgeInsets.all(8.0),
                child: Text(
                  answer,
                  textScaleFactor: 1.2,
                ),
              ),
            ],
          ),
        )
    );

    ListTile _tile(String title, String subtitle, IconData icon) => ListTile(
      title: Text(title,
          style: TextStyle(
            fontWeight: FontWeight.w500,
            fontSize: 20,
          )),
      subtitle: Text(subtitle),
      leading: Icon(
        icon,
        color: Colors.blue[500],
      ),
    );

    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Qalculate'),
        ),
        /*body: Center(

          child: Text('$command = $answer $msg'),// / ${nativeExecute()}
        ),*/
        body: GridView.extent(
          maxCrossAxisExtent: 150,
          children: <Widget>[
            main, Image.asset("images/perspective.png"),
            Image.asset("images/perspective.png"),
            Image.asset("images/perspective.png"),
            Image.asset("images/perspective.png"),
            Image.asset("images/perspective.png"),
          ],
        )
      ),
    );
  }
}
