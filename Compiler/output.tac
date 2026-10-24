x = 1
sum = 0
L1:
t1 = x < 6
ifFalse t1 goto L2
t2 = x * x
t3 = sum + t2
sum = t3
t4 = x + 1
x = t4
goto L1
L2:
print sum
t5 = sum > 50
ifFalse t5 goto L3
print 1
goto L4
L3:
print 0
L4:
