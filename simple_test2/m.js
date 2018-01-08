var ad=require('./build/Release/addon');
//var ad=addon();
ad.add(1,2,function(err,sum){console.log('err: ',err);console.log('sum: ',sum);})
ad.add(2,2,function(err,sum){console.log('err: ',err);console.log('sum: ',sum);})
ad.add(4,4,function(err,sum){console.log('err: ',err);console.log('sum: ',sum);})