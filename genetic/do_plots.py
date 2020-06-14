import matplotlib.pyplot as plt
import numpy as np
import sys

from collections import defaultdict

# if u wanna test local results
# res_directory = './results/'

# many cities (more complex single computation i.e: large graphs)
res_directory = './results-remote/xeon-results-10-1024-10000/'

# many chromosomes (less complex single computation i.e: less large graphs but more tasks subdivision)
# res_directory = './results-remote/xeon-results-10-16384-1000/'


def speedup(tseq, tpar):
  return round(((float)(tseq))/tpar, 3)

def scalability(tpar1, tpar):
  return round(((float)(tpar1))/tpar, 3)

def efficiency(tseq, p, tpar):
  return round(((float)(tseq))/(p*tpar), 3)


# load t_seq from file.
with open(res_directory+'t_seq.data') as f_tseq:
  tseq_data = f_tseq.readlines()

# load t_par from file.
with open(res_directory+'t_par.data') as f_tpar:
  tpar_data = f_tpar.readlines()

# load t_pool from file.
with open(res_directory+'t_pool.data') as f_tpool:
  tpool_data = f_tpool.readlines()

# load t_ff from file.
with open(res_directory+'t_ff.data') as f_tff:
  tff_data = f_tff.readlines()


# managing t_seq data
# extract only digits from read lines
tseq_data = [float(((l.strip()).split('='))[1]) for l in tseq_data]

# take the average
tseq_data = np.average(tseq_data)


# ************************************************************************************
# managing t_par data. Goal: build a list [(nw, t_par)] where t_par is an average time
# ************************************************************************************

tpar_data = [(l.strip()).split('=') for l in tpar_data] # ['t_par(1)', '4456']
# extract the ditig part from the list of strings of the form [] ['t_par(1)']
nw_list = []
for e in [ i[0] for i in tpar_data ]:
    nw_list.append(list(filter(str.isdigit, e)))
nw_list = [ int(''.join(nw_list[i])) for i in range(len(nw_list)) ]

tpar_data = sorted(list(zip( nw_list, [ int(i[1]) for i in tpar_data])), key=lambda x: x[0]) 


# produce a list [(nw_1, [tpar(nw_1), ..]), (nw_2, [tpar(nw_2), ..]), ..]
d = defaultdict(list)
for k, v in tpar_data:
    d[k].append(v)
tpar_data = list(d.items())


# finally tpar_data is a list of tuples [(nw, average tpar(nw)), ..]
tpar_data = sorted([(tpar_data[i][0], np.average(tpar_data[i][1])) for i in range(len(tpar_data))], key=lambda x: x[0])


# **************************************************************************
# NOW we apply the same procedure on data from pool implementation experiments
# **************************************************************************
tpool_data = [(l.strip()).split('=') for l in tpool_data] # ['tpool(1)', '4456']
# extract the ditig part from the list of strings of the form [] ['tpool(1)']
nw_list = []
for e in [ i[0] for i in tpool_data ]:
    nw_list.append(list(filter(str.isdigit, e)))
nw_list = [ int(''.join(nw_list[i])) for i in range(len(nw_list)) ]

tpool_data = sorted(list(zip( nw_list, [ int(i[1]) for i in tpool_data])), key=lambda x: x[0]) 


# produce a list [(nw_1, [tpool(nw_1), ..]), (nw_2, [tpool(nw_2), ..]), ..]
d = defaultdict(list)
for k, v in tpool_data:
    d[k].append(v)
tpool_data = list(d.items())


# finally tpool_data is a list of tuples [(nw, average tpool(nw)), ..]
tpool_data = sorted([(tpool_data[i][0], np.average(tpool_data[i][1])) for i in range(len(tpool_data))], key=lambda x: x[0])


# **************************************************************************
# NOW we apply the same procedure on data from ff implementation experiments
# **************************************************************************

tff_data = [(l.strip()).split('=') for l in tff_data] # ['t_ff(1)', '4456']

# extract the ditig part from the list of strings of the form [] ['t_ff(1)']
nw_list = []
for e in [ i[0] for i in tff_data ]:
    nw_list.append(list(filter(str.isdigit, e)))
nw_list = [ int(''.join(nw_list[i])) for i in range(len(nw_list)) ]

# sort the list by nw
tff_data = sorted(list(zip( nw_list, [ int(i[1]) for i in tff_data])), key=lambda x: x[0]) 

# produce a list [(nw_1, [tff(nw_1), ..]), (nw_2, [tff(nw_2), ..]), ..]
d = defaultdict(list)
for k, v in tff_data:
    d[k].append(v)
tff_data = list(d.items())

# finally tff_data is a list of tuples [(nw, average tff(nw)), ..]
tff_data = sorted([(tff_data[i][0], np.average(tff_data[i][1])) for i in range(len(tff_data))], key=lambda x: x[0])


# before plotting store ideal parallel times:
nw_list = [tff_data[i][0] for i in range(len(tff_data))]

tideal_data = sorted([(nw_list[i], tseq_data/nw_list[i]) for i in range(len(nw_list))], key=lambda x: x[0])

print(tpar_data)
print(tff_data)
print(tideal_data)

# now it's time to plot


# *** SPEEDUP ***
plt.title('Speedup')
plt.xlabel('p')
plt.ylabel('s(p)')
plt.grid(True)
# par
x,y = zip(*tpar_data)
y = [speedup(tseq_data, y[i]) for i in range(len(y))]
plt.xticks(x)
plt.axis([1, max(x), 0, max(y)*3])
plt.plot(x, y, marker='x', label='c++ threads')
# pool
x,y = zip(*tpool_data)
y = [speedup(tseq_data, y[i]) for i in range(len(y))]
plt.xticks(x)
plt.plot(x, y, marker='p', label='c++ threads pool')
# ff
x,y = zip(*tff_data)
y = [speedup(tseq_data, y[i]) for i in range(len(y))]
plt.plot(x, y, marker='*', label='fastflow')
# ideal
x,y = zip(*tideal_data)
y = [speedup(tseq_data, y[i]) for i in range(len(y))]
plt.plot(x, y, 'k--', label='ideal')

plt.legend(loc="upper right")
plt.show()


# *** SCALABILITY ***
plt.title('Scalability')
plt.xlabel('p')
plt.ylabel('scalab(p)')
plt.grid(True)
# par
x,y = zip(*tpar_data)
plt.xticks(x)
y = [scalability(y[0], y[i]) for i in range(len(y))]
plt.axis([1, max(x), 0, max(y)*2])
plt.plot(x, y, marker='x', label='c++ threads')
# pool
x,y = zip(*tpar_data)
plt.xticks(x)
y = [scalability(y[0], y[i]) for i in range(len(y))]
plt.axis([1, max(x), 0, max(y)*2])
plt.plot(x, y, marker='P', label='c++ threads pool')
# ff
x,y = zip(*tff_data)
y = [scalability(y[0], y[i]) for i in range(len(y))]
plt.plot(x, y, marker='*', label='fastflow')
# ideal
x,y = zip(*tideal_data)
y = [scalability(y[0], y[i]) for i in range(len(y))]
plt.plot(x, y, 'k--', label='ideal')

plt.legend(loc="upper right")
plt.show()


# EFFICIENCY
plt.title('Efficiency')
plt.xlabel('p')
plt.ylabel(r'$\epsilon(p)$')
plt.grid(True)
# par
x,y = zip(*tpar_data)
plt.xticks(x)
y = [efficiency(tseq_data, x[i], y[i]) for i in range(len(y))]
plt.axis([1, max(x), 0, max(y)*2])
plt.plot(x, y, marker='x', label='c++ threads')
# pool
x,y = zip(*tpool_data)
plt.xticks(x)
y = [efficiency(tseq_data, x[i], y[i]) for i in range(len(y))]
plt.axis([1, max(x), 0, max(y)*2])
plt.plot(x, y, marker='P', label='c++ threads pool')
# ff
x,y = zip(*tff_data)
y = [efficiency(tseq_data, x[i], y[i]) for i in range(len(y))]
plt.plot(x, y, marker='*', label='fastflow')

plt.legend(loc="upper right")
plt.show()
