#include <LittleFS.h>
#include <threadSafeFS.h> 
using File = threadSafeFS::File;    // Use thread-safe wrapper for all file operations from now on in your code


// 1️⃣ Crete thread-safe wrapper arround LittleFS (or SPIFFS or FFat or SD)
threadSafeFS::FS TSFS (LittleFS);

// 2️⃣ Include keyValueDatabase class and create an instance using TSFS
#include <keyValueDatabase.hpp>
keyValueDatabase<int, String> kvdb (TSFS);  // database instance where keys are integers and values are Strings (in this example)


void setup () {
    Serial.begin (115200);
    while (!Serial)
        delay (10);
    delay (3000);


    // 3️⃣ Mount file system
    LittleFS.begin (true);
    // if (LittleFS.format ()) Serial.println ("LittleFs formatted"); else Serial.println ("LittleFs formatting failed");

    signed char e;


    // 4️⃣ Open key-value database: load index values from the data file to balanced binary search tree
    Serial.println ("4️⃣ Open key-value database");
    e = kvdb.Open ("/example.kvdb");
    if (!e) //  OK
        Serial.printf ("   initially loaded %i key-value pairs\n", kvdb.size ());
    else
        Serial.printf ("   failed to load data, check errorFalgs () for details\n");


    // truncate (delete) all key-value pairs in case there are some already in kvdb
    // just for this example to start with an empty database
    kvdb.Truncate ();
    Serial.println ("key-value database truncated");


    // 5️⃣ Example of inserting new key-value pairs
    Serial.println ("5️⃣ a) Example of inserting new key-value pairs");
    e = kvdb.Insert (7, "seven");
    if (e) // != OK
        Serial.printf ("   kvdb.Insert failed, check errorFalgs () for details\n");
    e = kvdb.Insert (8, "eigth");
    if (e) // != OK
        Serial.printf ("   kvdb.Insert failed, check errorFalgs () for details\n");
    e = kvdb.Insert (9, "nine");
    if (e) // != OK
        Serial.printf ("   kvdb.Insert failed, check errorFalgs () for details\n");
    if (!kvdb.errorFlags ())
        Serial.printf ("   inserted %i key-value pairs\n", kvdb.size ());

    // --- alternativelly: ---
    kvdb.clearErrorFlags ();
    Serial.println ("5️⃣ b) Example of inserting new key-value pairs");
    kvdb [10] = "ten";
    if (kvdb.errorFlags ())
        Serial.println ("   insert of 10 failed");
    else
        Serial.println ("   inserted key-value pair");


    // 6️⃣ Examples of finding a key in key-value database
    Serial.println ("6️⃣ Examples of finding a key in key-value database");
    uint32_t blockOffset; // not needed for searching for a key, but will coma handy later when searching for a value
    e = kvdb.FindBlockOffset (8, blockOffset);
    switch (e) {
        case err_ok:        Serial.printf ("   key 8 found\n"); 
                            break;
        case err_not_found: Serial.printf ("   key 8 err_not_found\n"); 
                            break;
        default:            Serial.printf ("   FindBlockOffset error: %i\n", e); 
                            break;
    }


    // 7️⃣ Examples of finding a value belonging to a given key
    Serial.println ("7️⃣ a) Examples of finding a value belonging to a given key");
    String value;
    e = kvdb.FindValue (8, &value); 
    switch (e) {
        case err_ok:        Serial.printf ("   value for a key 8 found: %s\n", value.c_str ()); 
                            break;
        case err_not_found: Serial.printf ("   key 8 err_not_found\n"); 
                            break;
        default:            Serial.printf ("   FindValue error: %i\n", e); 
                            break;
    }    

    // --- alternativelly: ---
    Serial.println ("7️⃣ b) Examples of finding a value belonging to a given key");
    value = kvdb [9];
    if (value > "") { // or: if (!kvdb.errorFlags ())
        Serial.print ("   9 found: "); Serial.println (value);
    } else {
        Serial.println ("   9 not found");
    }
    

    // 8️⃣ Example of deleting a key-value pair from key-value database
    Serial.println ("8️⃣ Example of deleting a key-value pair from key-value database");
    e = kvdb.Delete (7); 
    if (e) // != OK
        Serial.printf ("   kvdb.Delete failed, check errorFalgs () for details\n");
    else
        Serial.println ("   OK");


    // 9️⃣ Example of updating the value for a given key
    Serial.println ("9️⃣ a) Example of updating the value for a given key");
    // update (a) the value (most simple version of calling update)
    e = kvdb.Update (8, "eight"); 
    if (e) // != OK
        Serial.printf ("   kvdb.Update failed, check errorFalgs () for details\n");
    else
        Serial.println ("   OK");


    Serial.println ("9️⃣ b) Example of updating the value with calculation from previous value");
    // update (b) the value with calculation (update with locking to prevent other tasks changind the data while the calculation is not finished)
    kvdb.Lock ();
        String oldValue;
        e = kvdb.FindValue (8, &oldValue); 
        if (e == err_ok)
            e = kvdb.Update (8, oldValue + "teen"); 
    kvdb.Unlock ();
    if (e) // != OK
        Serial.printf ("   kvdb.Update failed, check errorFalgs () for details\n");
    else
        Serial.println ("   OK");


    Serial.println ("9️⃣ c) Example of updating the value with calculation using lambda function");
    // update (c) the value with calculation using lambda callback function, the locking is already integrated so the calculation can be performed without problems
    // this mechanism is usefull for example for counters (increasing the value), etc)
    e = kvdb.Update (9, [] (String& value) { value.toUpperCase (); } ); 
    if (e) // != OK
        Serial.printf ("   kvdb.Update failed, check errorFalgs () for details\n");
    else
        Serial.println ("   OK");

    Serial.println ("9️⃣ d) Example of updating the value with [] operator");
    // --- alternativelly: ---
    kvdb.clearErrorFlags ();
    kvdb [10] = "TEN";
    if (kvdb.errorFlags ()) 
        Serial.println ("   update of 10 failed");
    else
        Serial.println ("   OK");

    Serial.println ("9️⃣ e) Example of updating the values (with calculation) during iteration using lambda function");
    // Please note that database is already locked throughtout the iterator, so additional locking is not needed.
    for (auto& p: kvdb) {
        e = kvdb.Update (p.key, [] (String& value) { value = "»" + value + "«"; }, &(p.blockOffset)); // since block offset is already known it will speed up Update operation if we provide this information
        if (e) 
            Serial.printf ("   Update error, check errorFalgs () for details\n");
    }
    if (!kvdb.errorFlags ()) 
        Serial.println ("   OK");    


    // 🔟 Example of iterating through all key-value pairs
    Serial.println ("🔟 Example of iterating through all key-value pairs");
    for (auto& p: kvdb) {
        // keys are always kept in memory and are obtained fast
        Serial.print ("   key:"); Serial.print (p.key); Serial.print (", blockOffset:"); Serial.print (p.blockOffset); Serial.print (", value:"); 
        
        // values are read from disk, obtaining a value may be much slower
        String value;
        e = kvdb.FindValue (p.key, &value, p.blockOffset); // blockOffset is optional but since we already have it we can speed up the search a bit by providing it
        if (e) // != OK 
            Serial.printf ("   FindValue error, check errorFalgs () for details\n");
        else
            Serial.println (value);
    }


    // 1️⃣1️⃣ Detecting errors that occured in key-value database operations
    Serial.println ("1️⃣1️⃣ Detecting errors that occured in key-value database operations");
    for (auto& p: kvdb) {
        String value;
        e = kvdb.FindValue (p.key, &value, p.blockOffset);
        if (!e) { // OK
            Serial.print ("   "); Serial.print (p.key); Serial.print (" - "); Serial.println (value);
        } else {
            Serial.printf ("   FindValue error while fetching a value from disk: ");
            switch (e) {
                  case err_bad_alloc:       Serial.printf ("err_bad_alloc\n"); break;
                  case err_not_found:       Serial.printf ("err_not_found\n"); break;
                  case err_not_unique:      Serial.printf ("err_not_unique\n"); break;
                  case err_data_changed:    Serial.printf ("err_data_changed\n"); break;
                  case err_file_io:         Serial.printf ("err_file_io\n"); break;
                  case err_cant_do_it_now:  Serial.printf ("err_cant_do_it_now\n"); break;
              }
        }
    }


    // 1️⃣2️⃣ Checking errors only once after multiple operations
    Serial.println ("1️⃣2️⃣ Checking errors only once after multiple operations");
    kvdb.clearErrorFlags ();
    for (int i = 1000; i < 1100; i++)
        kvdb.Insert (i, String (i));


    // 1️⃣3️⃣ Find first (last) keys
    Serial.println ("1️⃣3️⃣ Find first (last) keys");
    auto firstElement = first_element (kvdb);
    if (firstElement) // check if first element is found (if kvdb is not empty)
        Serial.printf ("   first element (min key) of kvdb = %i\n", (*firstElement).key);

    auto lastElement = last_element (kvdb);
    if (lastElement != kvdb.end ()) // check if last element is found (if kvdb is not empty)
        Serial.printf ("   last element (max key) of kvdb = %i\n", (*lastElement).key);


    // 1️⃣4️⃣ Capacity and speed test
    Serial.println ("1️⃣4️⃣ Capacity and speed test");    
    keyValueDatabase<unsigned long, String> test (TSFS);
    test.Open ("/test.kvdb");
    if (test.errorFlags ()) // != OK 
        Serial.printf ("   test failed to load data: %i, all the data may not be indexed\n", test.errorFlags ());
    else
        Serial.printf ("   test initially loaded %i key-value pairs\n", test.size ());
    test.Truncate ();
    Serial.printf ("   test truncted\n");
    test.clearErrorFlags ();
    unsigned long l;
    unsigned long startMillis = millis ();
    for (l = 1; l <= 100000; l++) {
        if (test.Insert ( l, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." ))
            break;
    if (l % 100 == 0)
        Serial.printf ("   %i ... %lu Bytes ...\n", test.size (), test.dataFileSize ());
    }
    e = test.errorFlags ();
    if (e) { // != OK
        if (e & err_bad_alloc)      Serial.println ("   err_bad_alloc");
        if (e & err_not_found)      Serial.println ("   err_not_found");
        if (e & err_not_unique)     Serial.println ("   err_not_unique");
        if (e & err_data_changed)   Serial.println ("   err_data_changed");
        if (e & err_file_io)        Serial.println ("   err_file_io");
        if (e & err_cant_do_it_now) Serial.println ("   err_cant_do_it_now");
    }
    test.Truncate ();
    unsigned long endMillis = millis ();

    Serial.printf ("   Maximum number of keyValueDatabase<unsigned long, String> in the memory or data file (whichever error occured) is %lu\n", l); 
    Serial.printf ("   Average Insert time = %lu ms\n", (endMillis - startMillis) / l);
}

void loop () {

}
