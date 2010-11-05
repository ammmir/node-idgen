/**
 * idgen.cc
 * A node.js module that generates unique IDs.
 *
 * Derived from Twitter's Snowflake ID generation implementation.
 *
 * Copyright (C) 2010 Amir Malik.
 * Licensed under the MIT license.
 */

// Based on the sample extension skeleton from https://www.cloudkick.com/blog/2010/aug/23/writing-nodejs-native-extensions/
// ID generation algorithm lifted from https://github.com/twitter/snowflake/blob/master/src/main/scala/com/twitter/service/snowflake/IdWorker.scala

#include <v8.h>
#include <node.h>
#include <time.h>
#include <iostream>

using namespace node;
using namespace v8;

class IdWorker: ObjectWrap
{
private:
  int m_count;
  int m_counter;

  unsigned long sequence;
  unsigned long workerIdBits;
  unsigned long datacenterIdBits;
  unsigned long maxWorkerId;
  unsigned long maxDatacenterId;
  unsigned long sequenceBits;

  unsigned long workerIdShift;
  unsigned long datacenterIdShift;
  unsigned long timestampLeftShift;
  unsigned long sequenceMask;

  unsigned long lastTimestamp;

protected:
  unsigned long m_epoch;
  unsigned long m_worker_id;
  unsigned long m_datacenter_id;
public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target)
  {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    s_ct->SetClassName(String::NewSymbol("IdWorker"));

    NODE_SET_PROTOTYPE_METHOD(s_ct, "nextId", IDGen);

    target->Set(String::NewSymbol("IdWorker"),
                s_ct->GetFunction());
  }

  IdWorker(unsigned long epoch, unsigned long workerId, unsigned long datacenterId) :
    m_count(0),
    m_counter(0)
  {
    sequence = 0L;
    workerIdBits = 5;
    datacenterIdBits = 5;
    maxWorkerId = -1L ^ (-1L << workerIdBits);
    maxDatacenterId = -1L ^ (-1L << datacenterIdBits);
    sequenceBits = 12;

    workerIdShift = sequenceBits;
    datacenterIdShift = sequenceBits + workerIdBits;
    timestampLeftShift = sequenceBits + workerIdBits + datacenterIdBits;
    sequenceMask = -1L ^ (-1L << sequenceBits);

    lastTimestamp = -1L;

    m_epoch = epoch;
    m_worker_id = workerId;
    m_datacenter_id = datacenterId;
  }

  ~IdWorker()
  {
  }

  unsigned long timeGen()
  {
    time_t now = time(NULL) * 1000;
    return now;
  }

  unsigned long tilNextMillis(unsigned long lastTs)
  {
    unsigned long timestamp = timeGen();

    while(lastTs == timestamp) {
      timestamp = timeGen();
    }

    return timestamp;
  }

  unsigned long nextId()
  {
      // TODO: sanity checks

      unsigned long timestamp = timeGen();

      if(lastTimestamp > timestamp)
        ; // TODO: clock skew forwards

      if(lastTimestamp == timestamp) {
        sequence = (sequence + 1) & sequenceMask;
        if(sequence == 0) {
          timestamp = tilNextMillis(lastTimestamp);
        }
      } else {
        sequence = 0;
      }

      lastTimestamp = timestamp;
      m_counter++;

      unsigned long id = ((timestamp - m_epoch) << timestampLeftShift) | (m_datacenter_id << datacenterIdShift) | (m_worker_id << workerIdShift) | sequence;
      std::cout << "new id: " << id << "\n";
      return id;
  }

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;

    if(args.Length() != 3)
      return ThrowException(Exception::TypeError(
            String::New("IdWorker(epoch, workerId, datacenterId) constructor expected")));

    if(!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString())
      return ThrowException(Exception::TypeError(
            String::New("IdWorker constructor needs String arguments")));

    Local<String> epoch = Local<String>::Cast(args[0]);
    Local<String> workerId = Local<String>::Cast(args[0]);
    Local<String> datacenterId = Local<String>::Cast(args[0]);

    char buf[32];
    unsigned long l_epoch, l_worker_id, l_datacenter_id;

    epoch->WriteAscii(&buf[0], 0, 31);
    l_epoch = atol(buf);

    workerId->WriteAscii(&buf[0], 0, 31);
    l_worker_id = atol(buf);

    datacenterId->WriteAscii(&buf[0], 0, 31);
    l_datacenter_id = atol(buf);

    IdWorker* hw = new IdWorker(l_epoch, l_worker_id, l_datacenter_id);
    hw->Wrap(args.This());
    return args.This();
  }

  static char* ltoa(unsigned long n, char *s, int radix) {
    long q, r;
    long i = 0, j;
    char tmp[65]; // worst case: base-2 of a 64-bit positive integer

    do {
      q = long(n / radix);
      r = n % radix;
      n = q;
      tmp[i++] = 48 + r; // 48 is decimal for ASCII 0
    } while(q > 0);

    for(j = 0; j < i; j++) {
      s[j] = tmp[i - j - 1];
    }

    s[j] = '\0';

    return s;
  }

  static Handle<Value> IDGen(const Arguments& args)
  {
    HandleScope scope;
    IdWorker* hw = ObjectWrap::Unwrap<IdWorker>(args.This());
    hw->m_count++;

    //Local<String> result = String::New("Hello World");
    //Local<Number> result = Number::New(hw->m_epoch);
    //Local<Number> result = Number::New( Date::New(now)->NumberValue() );
    //Local<Number> result = Number::New( hw->nextId() );

    unsigned long id = hw->nextId();

    //Local<Number> result = Number::New(id);
    //Handle<Object> result = Persistent<String>::New( String::Cast( Number::New(id) ) );
    //Local<String> result = Local<String>::Cast( Number::New(id) );

    char buf[65];
    ltoa(id, &buf[0], 10);

//    Handle<String> result = Persistent<String>::New( String::New(buf) );

    Local<String> result = Local<String>::Cast( Number::New(id) );
    return scope.Close(result);
  }

};

Persistent<FunctionTemplate> IdWorker::s_ct;

extern "C" {
  static void init (Handle<Object> target)
  {
    IdWorker::Init(target);
  }

  NODE_MODULE(idgen, init);
}
