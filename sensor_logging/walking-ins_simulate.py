import csv
import numpy as np
import matplotlib.pyplot as plt


STOP_ACCELERATION_MAX = 5.0
STOP_GYRO_MAX = 100
STOP_TIME_MIN = 50
MOVE_TIME_MIN = 25

CALIB_ACCELERATION_MAX = 0.8
CALIB_GYRO_MAX = 35
CALIB_GYRO_RANGE = 1.0
CALIB_INTERVAL_MIN = 500
CALIB_TIME_MIN = 40


plot_size = (40, 6)
data_headers = []
data_dict = {}
start_time = -1
with open('datastream.csv', encoding='utf-8') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for line in csv_reader:
        if len(data_headers) == 0:
            for header in line:
                data_headers.append(header)
                data_dict[header] = []
        else:
            for i in range(len(line)):
                if i == 0:
                    if start_time == -1:
                        start_time = int(line[i])
                    data_dict[data_headers[i]].append(int(line[i]) - start_time)
                else:
                    data_dict[data_headers[i]].append(float(line[i]))

data_dict['acc_norm'] = np.abs(9.825 - np.sqrt(np.power(data_dict['acc_x'], 2.0) + np.power(data_dict['acc_y'], 2.0)
                                               + np.power(data_dict['acc_z'], 2.0)))
data_dict['gyro_norm'] = np.abs(np.sqrt(np.power(data_dict['gyro_x'], 2.0) + np.power(data_dict['gyro_y'], 2.0)
                                        + np.power(data_dict['gyro_z'], 2.0)))

moving_t = [data_dict['time'][0]]
moving_val = [0]
calib_t = [data_dict['time'][0]]
calib_val = [0]
t_stop = 0
t_move = 0
t_calib = 0
calib_gyro_max = 0
calib_gyro_min = 0
last_calib_min = 0
last_calib_t = 0
for i in range(len(data_dict['time'])):
    if data_dict['acc_norm'][i] <= STOP_ACCELERATION_MAX and data_dict['gyro_norm'][i] <= STOP_GYRO_MAX:
        if t_stop == 0:
            t_stop = data_dict['time'][i]
        elif data_dict['time'][i] - t_stop >= STOP_TIME_MIN:
            moving_t.append(t_stop - 1)
            moving_val.append(moving_val[-1])
            moving_t.append(t_stop)
            moving_val.append(0)
            t_move = 0

        if data_dict['acc_norm'][i] <= CALIB_ACCELERATION_MAX and data_dict['gyro_norm'][i] <= CALIB_GYRO_MAX\
            and calib_gyro_max - calib_gyro_min <= CALIB_GYRO_RANGE\
            and (data_dict['time'][i] - last_calib_t >= CALIB_INTERVAL_MIN
                 or data_dict['gyro_norm'][i] < last_calib_min):
            if t_calib == 0:
                t_calib = data_dict['time'][i]

                calib_gyro_max = data_dict['gyro_norm'][i]
                calib_gyro_min = data_dict['gyro_norm'][i]
            elif data_dict['time'][i] - t_calib >= CALIB_TIME_MIN:
                calib_t.append(t_calib - 1)
                calib_val.append(calib_val[-1])
                calib_t.append(t_calib)
                calib_val.append(100)

                calib_gyro_max = max(calib_gyro_max, data_dict['gyro_norm'][i])
                calib_gyro_min = min(calib_gyro_min, data_dict['gyro_norm'][i])
        else:
            if calib_val[-1] == 100:
                last_calib_t = data_dict['time'][i]
                last_calib_min = calib_gyro_min

            calib_t.append(data_dict['time'][i] - 1)
            calib_val.append(calib_val[-1])
            calib_t.append(data_dict['time'][i])
            calib_val.append(0)
            calib_gyro_max = 0
            calib_gyro_min = 0
            t_calib = 0
    else:
        if t_move == 0:
            t_move = data_dict['time'][i]
        elif data_dict['time'][i] - t_move >= MOVE_TIME_MIN:
            moving_t.append(data_dict['time'][i - 1])
            moving_val.append(moving_val[-1])
            moving_t.append(data_dict['time'][i])
            moving_val.append(100)
            t_stop = 0

moving_t.append(data_dict['time'][-1])
moving_val.append(moving_val[-1])
calib_t.append(data_dict['time'][-1])
calib_val.append(calib_val[-1])

fig, ax1 = plt.subplots(figsize=plot_size, dpi=300)

color = 'tab:red'
ax1.set_xlabel('time (ms)')
ax1.set_ylabel('acc_norm', color=color)
ax1.plot(data_dict['time'], data_dict['acc_norm'], color=color)
ax1.tick_params(axis='y', labelcolor=color)
plt.xticks(np.arange(data_dict['time'][0], data_dict['time'][-1]+1, step=1000))

ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

color = 'tab:blue'
ax2.set_ylabel('gyro_norm', color=color)  # we already handled the x-label with ax1
ax2.plot(data_dict['time'], data_dict['gyro_norm'], color=color)
ax2.plot(moving_t, moving_val, color='orange')
ax2.plot(calib_t, calib_val, color='green')
ax2.tick_params(axis='y', labelcolor=color)

fig.tight_layout()  # otherwise the right y-label is slightly clipped
plt.savefig('datastream.png')
#plt.show()
