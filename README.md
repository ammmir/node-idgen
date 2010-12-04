# idgen

idgen is a [node.js](http://nodejs.org/) module that implements Twitter's
[Snowflake](https://github.com/twitter/snowflake) ID generation scheme.

It's pretty much a straight port of the IdWorker.scala guts. Of course, it
could very well be completely wrong, and most likely is until the TODOs are
addressed.

## Usage

Build it like any other node.js module:

    node-waf configure build

Generate some IDs:

    var util = require('util'),
        IdWorker = require('./build/default/idgen').IdWorker;

    var epoch        = '1288834974657',
        workerId     = '1',
        datacenterId = '1',
        idgen        = new IdWorker(epoch, workerId, datacenterId);

    util.puts('new id: ' + idgen.nextId());
    // ...

## TODO

* There are some memory leaks caused by my newbness to V8
* Ensure this implementation is consistent with Snowflake for a given set of
  seed values
