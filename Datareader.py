with open('E:\\DATALOG.TXT', 'r+') as phil:
    mass = float(input("Enter vehicle mass in kilograms: "))
    gas = float(input("Enter gallons of gasoline consumed: "))
    energyIn = gas *130000000
    work = 0.0
    nwork = 0.0
    dist = 0.0
    phil.seek(0,0)
    for line in phil:
        line = line[0:len(line)-1]
        words = line.split(',')
        if len(words) >= 10:
##          The "+ .07 <" here corresponds to the orientation
##          of the accelerometer relative to the vehicle
            if float(words[1]) + .07 < 0:
                work += (float(words[1])+.07)*float(words[4])*9.8*mass
            else:
                nwork += (float(words[1])+.07)*float(words[4])*9.8*mass
##              print(str(work) + "J")
            dist += float(words[4])
##              print(str(dist)+ "m" )
        else:
            print("Error: Corrupt line")

phil.closed

print("Positive work: " + str(-work) + "J")
print("Negative work: " + str(-nwork) + "J")
print("Energy consumed: " + str(energyIn) + "J")
print("Efficiency: " + str(-work/energyIn*100) + "%")
print("Distance traveled: " + str(dist) + "m")

