''' FUNTION TO FILTER THE FILE  '''

print("test")
import os,glob
i_path = 'C:\\Users\\asjha2\\Desktop\\result1\\bootflash\\btlogs'
o_path = 'C:\\Users\\asjha2\\Desktop\\'

def filter(f, f_out):
    for line in f:
        temp = 1
        step = 1
        for letter in range(len(line)):
            if(step == 1):
                if(line[letter] == ']'):
                    temp = 1
                    step = step + 1
                    f_out.write(line[letter])
                    #print (letter)
                if(line[letter]== '[' or temp == 0):
                    temp = 0
                    f_out.write(line[letter])
            if (step == 2):
                if(line[letter] == ')'):
                    temp = 1
                    step = step + 1
                if(line[letter]== '(' or temp == 0):
                    temp = 0
                    f_out.write(line[letter])
            if (step == 3):
                f_out.write(line[letter])
    return 

for filename in glob.glob(os.path.join(i_path, '*.txt')):
    print (filename)
    f = open(filename,'r')
    f_out = open((filename.rsplit( ".", 1 )[ 0 ] ) + "_filter.txt", "w+")
    filter(f,f_out)
    f.close()
    f_out.close()
    print ("end")
    os.remove(filename)
    
    #print (f.read())
