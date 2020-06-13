import matplotlib.pyplot as plt
import numpy as np
import sys

from collections import defaultdict

def ideal_time(tseq, nw):
  return ((float)tseq) / nw


def speedup(tseq, tpar):
  return ((float)(tseq))/tpar

def scalability(tpar1, tpar):
  return ((float)(tpar1))/tpar

def efficiency(tseq, p, tpar):
  return ((float)(tseq))/(p*tpar)




# load t_seq from file.
with open('../results/t_seq.data') as f_tseq:
  tseq_data = f_tseq.readlines()

# load t_par from file.
with open('../results/t_par.data') as f_tpar:
  tpar_data = f_tpar.readlines()

# load t_ff from file.
with open('../results/t_ff.data') as f_tff:
  tff_data = f_tff.readlines()

# managing t_seq data
# extract only digits from read lines
tseq_data = [float(((l.strip()).split('='))[1]) for l in tseq_data]
# take the average
tseq_data = np.average(tseq_data)


# managing t_par data. Goal: build a list [(nw, t_par)] where t_par is an average time

tpar_data = [(l.strip()).split('=') for l in tpar_data] # ['t_par(1)', '4456']

# extract the ditig part from the list of strings of the form [] ['t_par(1)']
nw_list = []
for e in [ i[0] for i in tpar_data ]:
    nw_list.append(list(filter(str.isdigit, e)))
nw_list = [int(i[0]) for i in nw_list]

# sort the list by nw
tpar_data = sorted(list(zip( nw_list, [ int(i[1]) for i in tpar_data])), key=lambda x: x[0]) 

# produce a list [(nw_1, [tpar(nw_1), ..]), (nw_2, [tpar(nw_2), ..]), ..]
d = defaultdict(list)
for k, v in tpar_data:
    d[k].append(v)
tpar_data = list(d.items())

# finally tpar_data is a list of tuples [(nw, average tpar(nw)), ..]
tpar_data = [(tpar_data[i][0], np.average(tpar_data[i][1])) for i in range(len(tpar_data))]

# **************************************************************************
# NOW we apply the same procedure on data from ff implementation experiments
# **************************************************************************

tff_data = [(l.strip()).split('=') for l in tff_data] # ['t_ff(1)', '4456']

# extract the ditig part from the list of strings of the form [] ['t_ff(1)']
nw_list = []
for e in [ i[0] for i in tff_data ]:
    nw_list.append(list(filter(str.isdigit, e)))
nw_list = [int(i[0]) for i in nw_list]

# sort the list by nw
tff_data = sorted(list(zip( nw_list, [ int(i[1]) for i in tff_data])), key=lambda x: x[0]) 

# produce a list [(nw_1, [tff(nw_1), ..]), (nw_2, [tff(nw_2), ..]), ..]
d = defaultdict(list)
for k, v in tff_data:
    d[k].append(v)
tff_data = list(d.items())

# finally tff_data is a list of tuples [(nw, average tff(nw)), ..]
tff_data = [(tff_data[i][0], np.average(tff_data[i][1])) for i in range(len(tff_data))]

# now its time to plot

# SPEEDUP
# ideal

# par
x,y = zip(*tpar_data)
y = [speedup(tseq_data, y[i]) for i in range(len(y))]
plt.plot(x, y)
# ff
x,y = zip(*tff_data)
y = [speedup(tseq_data, y[i]) for i in range(len(y))]
plt.plot(x, y)

plt.show()

# SCALABILITY
# par
x,y = zip(*tpar_data)
y = [scalability(y[0], y[i]) for i in range(len(y))]
plt.plot(x, y)
# ff
x,y = zip(*tff_data)
y = [scalability(y[0], y[i]) for i in range(len(y))]
plt.plot(x, y)
plt.show()


# EFFICIENCY
# par
x,y = zip(*tpar_data)
y = [efficiency(tseq_data, x[i], y[i]) for i in range(len(y))]
plt.plot(x, y)
# ff
x,y = zip(*tff_data)
y = [efficiency(tseq_data, x[i], y[i]) for i in range(len(y))]
plt.plot(x, y)
plt.show()

n_exps = len(sizes)",
for i,n in enumerate(sizes):",
    ",
    plt.subplot(1,n_exps,i+1)",
    plt.title(\"TSP on \"+str(n)+\" cities\",fontsize=22)",
    plt.xscale(\"log\")",
    #plt.yscale(\"log\")",
    plt.hlines(data[n][0],0,1000,label=\"Seq time\")",
    plt.plot(data[n][1][:,0],data[n][1][:,1],'-o',label=\"Parallel time\")",
    #plt.ylim(100,650)",
    #plt.xlim(1,1024)",
    plt.xticks(size=22)",
    plt.yticks(size=22)",
    plt.grid(True)",
    plt.legend(fontsize=22)",
    plt.show()",
