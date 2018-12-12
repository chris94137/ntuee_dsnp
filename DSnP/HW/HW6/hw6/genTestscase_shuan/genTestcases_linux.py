from os import listdir
from os.path import isfile, join
import random
import math
import sys

dirPath = '../tests.' + str(sys.argv[1]) + '/'

onlyfiles = [f for f in listdir(dirPath) if isfile(join(dirPath, f))]
print(onlyfiles)

file = open('mydofile.' + str(sys.argv[1]), 'w')
for f in onlyfiles:
    file.write('cirr ' + dirPath + f + ' -r\n')
    file.write('cirp\n')
    file.write('cirp -n\n')
    file.write('cirp -pi\n')
    file.write('cirp -po\n')
    file.write('cirp -fl\n')
    file.write('cirw\n')

    if 'err' in f:
        continue
    if 'do' in f:
        continue
    if 'log' in f:
        continue

    aag = open(dirPath + f, 'r')
    aagContent = aag.readlines()

    # print(aagContent[0])
    m = int(aagContent[0].split()[1])
    i = int(aagContent[0].split()[2])
    o = int(aagContent[0].split()[4])
    a = int(aagContent[0].split()[5])
    _max = i + o + a + 1

    testcaseNo = int(10 * math.log(len(aagContent), 10)) + 10
    
    cirgList = []
    faninList1 = []
    faninList2 = []
    # fanoutList = []

    # print(f)

    for j in range(int(testcaseNo * 3 / 10)):
        cirgList.append(int(random.choice(aagContent[1:_max]).split()[0]) // 2)
        if i != 0:
            faninList1.append(int(random.choice(aagContent[i:_max]).split()[0]) // 2)
        if o != 0:
            faninList2.append(random.randint(m + 1, m + o))

    #for j in range(int(testcaseNo * 2 / 10)):
    #    fanoutList.append(int(random.choice(aagContent[1:_max]).split()[0]) // 2)

    for tc in cirgList:
        file.write('cirg ' + str(tc) + '\n')

    for tc in faninList1:
        file.write('cirg ' + str(tc) + ' -fani ' + str(random.randint(1, len(aagContent))) + '\n')

    for tc in faninList2:
        file.write('cirg ' + str(tc) + ' -fani ' + str(random.randint(1, len(aagContent))) + '\n')

    #for tc in fanoutList:
    #    file.write('cirg ' + str(tc) + ' -fano ' + str(random.randint(1, len(aagContent))) + '\n')

    aag.close()
file.write('usage\n')
file.write('q -f\n')
file.close()
