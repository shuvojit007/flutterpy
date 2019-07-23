package flutterdesign.learning.com.flutterpy;

import android.os.Bundle;
import android.util.Log;

import io.flutter.app.FlutterActivity;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugins.GeneratedPluginRegistrant;

public class MainActivity extends FlutterActivity {
 // static String platform = "flutter.shuvojit.com.channel";
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    GeneratedPluginRegistrant.registerWith(this);

//
//    new MethodChannel(getFlutterView(), platform).setMethodCallHandler(
//            (call, result) -> {
//              if (call.method.equals("callNative")) {
//                Log.d("PYOUT", "onNewLine: "+call.arguments.toString());
//
//                PyBridge pyBridge = PyBridge.getInstance();
//
//
//                AssetExtractor assetExtractor =new  AssetExtractor(this);
//                assetExtractor.removeAssets("python");
//                assetExtractor.copyAssets("python");
//
//
//                // Get the extracted assets directory
//                String pythonPath = assetExtractor.getAssetsDataDir() + "python";
//
//                pyBridge.start(pythonPath);
//
//                pyBridge.call(call.arguments.toString());
//
//                pyBridge.setPythonStdListener(new PyBridge.PythonStdListener() {
//                  @Override
//                  public void onInputWait() {
//
//                  }
//
//                  @Override
//                  public void onNewLine(String data) {
//                    result.success("Return data from Native  =>"+data);
//                    Log.d("PYOUT", "onNewLine: "+data);
//                  }
//                });
//
//                //result.success("Return data from Native  =>"+call.arguments.toString());
//              }
//            });
  }
}
