package mobitnt.android.YaSync;


import org.acra.*;
import org.acra.annotation.*;

import android.app.Application;

@ReportsCrashes(
    formKey = "", // This is required for backward compatibility but not used
    formUri = "http://a.mobitnt.com/acr.php"
)

public class AcraApp extends Application {
	 @Override
	    public void onCreate() {
	        super.onCreate();

	        // The following line triggers the initialization of ACRA
	        ACRA.init(this);
	    }

}


