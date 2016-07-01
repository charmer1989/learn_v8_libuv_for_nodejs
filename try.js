var fn = require('./build/Release/delaycb')
console.log('begin run')
fn(500, function (err, str) {
  if (err) return console.error(err)
  console.log('500 after delay str=', str)
})
console.log('mid run')
fn(1500, function (err, str) {
  if (err) return console.error(err)
  console.log('1000 after delay str=', str)
})
