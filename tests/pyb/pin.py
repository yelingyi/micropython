from pyb import Pin

p = Pin('X1')
print(p)
print(p.name())
print(p.pin())
print(p.port())

p = Pin('X1', Pin.IN, Pin.PULL_UP)
#p = Pin('X1', Pin.IN, pull=Pin.PULL_UP)
print(p.value())

p.init(p.IN, p.PULL_DOWN)
#p.init(p.IN, pull=p.PULL_DOWN)
print(p.value())

p.init(p.OUT_PP)
p.low()
print(p.value())
p.high()
print(p.value())
p.value(0)
print(p.value())
p.value(1)
print(p.value())
p.value(False)
print(p.value())
p.value(True)
print(p.value())
