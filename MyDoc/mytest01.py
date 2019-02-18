import os
import pygame

print("hello world!");
print("hello world!");
print("hello world!");

message = "who are you"
print(message);
print('python');
print('\tpython');  

#调用函数str(),它让Python将非字符串值表示为字符串
age = 23 
message = "Happy " + str(age) + "rd Birthday!" 
print(message)

#列表
bicycles = ['wo','le','ge','qu']
print(bicycles) #输出['trek', 'cannondale', 'redline', 'specialized']
#访问列表元素
print(bicycles[0])
print(bicycles[0].title())
#Python为访问最后一个列表元素提供了一种特殊语法。通过将索引指定为-1,索引-2 返回倒数第 二个列表元素，索引-3 返回倒数第三个列表元素，以此类推。 
print(bicycles[-1])
#在列表末尾添加元素 
bicycles.append('hello word') 
print(bicycles)
#确定列表的长度
print(len(bicycles)) 
#遍历整个列表 在for 循环后面，没有缩进的代码都只执行一次，而不会重复执行
magicians = ['alice', 'david', 'carolina'] 
for magician in magicians: #for 语句末尾的冒号告诉Python，下一行是循环的第一行。 
    print(magician)


#字典  在Python中，字典用放在花括号{} 中的一系列键—值对表
alien_0 = {'color': 'green', 'points': 5}


#类的定义及使用
class Car():
    """一次模拟汽车的简单尝试"""      
    def __init__(self, make, model, year):          
        self.make = make          
        self.model = model          
        self.year = year          
        self.odometer_reading = 0      
    def get_descriptive_name(self):          
        long_name = str(self.year) + ' ' + self.make + ' ' + self.model
        return long_name.title()      
    def read_odometer(self):          
        print("This car has " + str(self.odometer_reading) + " miles on it.")      
    def update_odometer(self, mileage):          
        if mileage >= self.odometer_reading:              
            self.odometer_reading = mileage          
        else:              
            print("You can't roll back an odometer!")      
            
    def increment_odometer(self, miles):          
        self.odometer_reading += miles

#类的继承       
class ElectricCar(Car):      
    """电动汽车的独特之处"""    
    def __init__(self, make, model, year):          
        """初始化父类的属性"""          
        super().__init__(make, model, year)    #的super() 是一个特殊函数，帮助Python将父类和子类关联起来，父类也称为超类 超类 （superclass），名称super因此而得名。 


my_tesla = ElectricCar('tesla', 'model s', 2016)  
print(my_tesla.get_descriptive_name())


print('\n')
#读取文件
with open('file\\test.txt') as file_object: #关键字with 在不再需要访问文件后将其关闭。在这个程序中，注意到我们调用了open() ，但没有调用close() 
    contents = file_object.read()
    print(contents)

#逐行读取文件
with open('file\\test.txt') as file_object:      
    for line in file_object:      
        print(line)

#写入文件
filename = 'file\\programming.txt' 
with open(filename, 'w') as file_object:
    file_object.write("I love programming.\n")    
    file_object.write("I love creating new games.\n")


#异常
try:    
    print(5/0) 
except ZeroDivisionError:        
    print("You can't divide by zero!")


#JSon
import json  
# 如果以前存储了用户名，就加载它  
# 否则，就提示用户输入用户名并存储它  
filename = 'username.json'  
try:      
    with open(filename) as f_obj:         
        username = json.load(f_obj)  
except FileNotFoundError:      
    username = input("What is your name? ")      
    with open(filename, 'w') as f_obj:          
        json.dump(username, f_obj)          
        print("We'll remember you when you come back, " + username + "!") 
else:      
    print("Welcome back, " + username + "!")

print (pygame.__version__)

os.system('pause')