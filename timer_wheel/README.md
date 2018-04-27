﻿# timer wheel
此练习基本是skynet时钟系统的c++版本，同时增加了取消定时器这一操作。区别有两点：

1.在skynet的版本中，定时器的位置是通过位判断，如：
>(time|TIME_NEAR_MASK)==(current_time|TIME_NEAR_MASK)

在练习的版本中，是通过时间差来判断，如：
>diff_ticks < TIME_NEAR

两者有着略微的差别。以时钟为例，假如现在是11:50:40，一个过期时间为11:51:30，在skynet版本中，定时器的位置是在分钟这个层级的，因为分钟的数值不同，而在练习的版本中，定时器的位置是在秒这个层级的，因为两者的时间是小于1分钟的。skynet的方案限制了当前时间的范围，只能在2^32范围内。

2.二维数组转变成了一维，用list保存所有定时器。因为增加了取消定时器的操作，所以增加了id->timer的映射关系，timer也保存了在list的位置信息。