var util = require('util');

var IdWorker = require('./build/default/idgen').IdWorker;

var epoch        = '1288834974657',
    workerId     = '1',
    datacenterId = '1';

var idgen = new IdWorker(epoch, workerId, datacenterId);

for(var i = 0; ; i++) {
  var id = idgen.nextId();

  if(i % 10000)
    util.puts(i + 'th id: ' + id);
}
