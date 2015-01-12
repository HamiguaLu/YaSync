package mobitnt.util;

import java.io.IOException;
import java.util.List;  
  
import org.apache.http.HttpResponse;  
import org.apache.http.HttpStatus;  
import org.apache.http.NameValuePair;  
import org.apache.http.client.HttpClient;  
import org.apache.http.client.entity.UrlEncodedFormEntity;  
import org.apache.http.client.methods.HttpGet;  
import org.apache.http.client.methods.HttpPost;  
import org.apache.http.impl.client.DefaultHttpClient;  
import org.apache.http.params.BasicHttpParams;  
import org.apache.http.params.HttpConnectionParams;  
import org.apache.http.params.HttpParams;  
import org.apache.http.util.EntityUtils;

import android.util.Log;
  
 

/*需要主要的是:

1.    使用POST方式时，传递参数必须使用NameValuePair数组
2.    使用GET方式时，通过URL传递参数，注意写法

3.      通过setEntity方法来发送HTTP请求

4.      通过DefaultHttpClient 的 execute方法来获取HttpResponse
5. 通过getEntity()从Response中获取内容*/
  
public class HttpRequestHelper {  
    /** 
     *Post请求 
     * @return 
     */  
    public String doPostEx(String url , List<NameValuePair> nameValuePairs){  
        //新建HttpClient对象    
        HttpClient httpclient = new DefaultHttpClient();  
        //创建POST连接  
        HttpPost httppost = new HttpPost(url);  
        try {  
            httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));  
            HttpResponse response = httpclient.execute(httppost);
            if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK){
                Log.i("POST", "Bad Request!");
                MobiTNTLog.write("doPostEx failed with code " + String.valueOf(response.getStatusLine().getStatusCode()));
                return "";
            }else{
            	String content = EntityUtils.toString(response.getEntity());
            	return content;
            }
        } catch (IOException e) {  
            e.printStackTrace();
            MobiTNTLog.write("doPostEx failed with exception " + e.toString());
        }  
        
        return "";
    }  
      
    /** 
     *Get请求 
     */  
    public void doGet(String url){  
        HttpParams httpParams = new BasicHttpParams();  
        HttpConnectionParams.setConnectionTimeout(httpParams,30000);    
        HttpConnectionParams.setSoTimeout(httpParams, 30000);    
              
        HttpClient httpClient = new DefaultHttpClient(httpParams);  
        // GET  
        HttpGet httpGet = new HttpGet(url);
        try {  
            HttpResponse response = httpClient.execute(httpGet);
            //String content = EntityUtils.toString(response.getEntity());
            if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK){
                Log.i("GET", "Bad Request!");
                MobiTNTLog.write("GET method failed");
            }  
        } catch (IOException e) {  
            e.printStackTrace();
            MobiTNTLog.write(e.toString());
        }  
  
    }  
    
    public String doGetEx(String url){  
        HttpParams httpParams = new BasicHttpParams();  
        HttpConnectionParams.setConnectionTimeout(httpParams,30000);    
        HttpConnectionParams.setSoTimeout(httpParams, 60000);    
              
        HttpClient httpClient = new DefaultHttpClient();  
        // GET  
        HttpGet httpGet = new HttpGet(url);
        try {  
            HttpResponse response = httpClient.execute(httpGet);
            
            if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK){
                Log.i("GET", "Bad Request!");
                MobiTNTLog.write("doGetEx failed with code " + String.valueOf(response.getStatusLine().getStatusCode()));
                return "";
            }else{
            	String content = EntityUtils.toString(response.getEntity());
            	return content;
            }
        } catch (IOException e) {
            e.printStackTrace();
            MobiTNTLog.write("doGetEx failed with exception " + e.toString());
            return "";
        }  
  
    }  
}  

