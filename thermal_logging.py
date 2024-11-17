# import pandas as pd

# file= "C:/Users/91982/Downloads/openhardwaremonitor-v0.9.6/OpenHardwareMonitor/OpenHardwareMonitorLog-2024-11-16.csv"
# df= pd.read_csv(file)
# print(df)


# target_columns = [
#         "/intelcpu/0/temperature/0",
#         "/intelcpu/0/temperature/1",
#         "/intelcpu/0/temperature/2",
#         "/intelcpu/0/temperature/3",
#         "/intelcpu/0/temperature/4",
#         "/intelcpu/0/temperature/5",
#         "/intelcpu/0/temperature/6"
#     ]

#     # Check if the columns exist in the DataFrame
# available_columns = [col for col in target_columns if col in df.columns]
    

#     # Extract the required columns and print them
# print("Extracted Temperature Data:")
# print(df[available_columns])
import pandas as pd
import numpy as np
import tensorflow
import matplotlib.pyplot as plt
from sklearn.preprocessing import MinMaxScaler
from sklearn.model_selection import train_test_split
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense


file_path = "extracted_temperatures.csv"  
data = pd.read_csv(file_path, skiprows=1)
data['Time'] = pd.to_datetime(data['Time'], format='%m/%d/%Y %H:%M:%S')
# extract useful features from the "Time" column
data['hour'] = data['Time'].dt.hour
data['day_of_week'] = data['Time'].dt.dayofweek  # Monday=0, Sunday=6
data['hour_sin'] = np.sin(2 * np.pi * data['hour'] / 24)
data['hour_cos'] = np.cos(2 * np.pi * data['hour'] / 24)
data['day_of_week_sin'] = np.sin(2 * np.pi * data['day_of_week'] / 7)
data['day_of_week_cos'] = np.cos(2 * np.pi * data['day_of_week'] / 7)


data.drop(columns=['Time', 'hour', 'day_of_week'], inplace=True)

temperature_columns = [
    'CPU Core #1', 'CPU Core #2', 'CPU Core #3', 'CPU Core #4',
    'CPU Core #5', 'CPU Core #6', 'CPU Package'
]
cyclic_features = ['hour_sin', 'hour_cos', 'day_of_week_sin', 'day_of_week_cos']
selected_columns = temperature_columns + cyclic_features
temperature_data = data[selected_columns]
scaler = MinMaxScaler(feature_range=(0, 1))
scaled_data = scaler.fit_transform(temperature_data)

# function to create sequences for time-series prediction
def create_sequences(data, sequence_length):
    x, y = [], []
    for i in range(len(data) - sequence_length):
        x.append(data[i:i + sequence_length])
        y.append(data[i + sequence_length, :len(temperature_columns)])  # Only predict temperature columns
    return np.array(x), np.array(y)

#set sequence length and create sequences
sequence_length = 10  
x, y = create_sequences(scaled_data, sequence_length)
x_train, x_test, y_train, y_test = train_test_split(x, y, test_size=0.2, random_state=42)

#lstm
model = Sequential([
    LSTM(64, input_shape=(sequence_length, x.shape[2]), return_sequences=True),
    LSTM(32),
    Dense(len(temperature_columns)) 
])

model.compile(optimizer='adam', loss='mse')
model.summary()
history = model.fit(x_train, y_train, epochs=20, batch_size=32, validation_data=(x_test, y_test))
loss = model.evaluate(x_test, y_test)
print(f"Test Loss: {loss:.4f}")
y_pred = model.predict(x_test)


y_test_rescaled = scaler.inverse_transform(
    np.hstack([y_test, x_test[:, -1, len(temperature_columns):]])
)[:, :len(temperature_columns)]
y_pred_rescaled = scaler.inverse_transform(
    np.hstack([y_pred, x_test[:, -1, len(temperature_columns):]])
)[:, :len(temperature_columns)]

# Plot predictions for all CPU cores
for i, column in enumerate(temperature_columns):
    plt.figure(figsize=(12, 6))
    plt.plot(y_test_rescaled[:, i], label=f"True {column}")
    plt.plot(y_pred_rescaled[:, i], label=f"Predicted {column}")
    plt.xlabel("Time Steps")
    plt.ylabel("Temperature")
    plt.title(f"True vs. Predicted Temperature for {column}")
    plt.legend()
    plt.show()

# Plot the training and validation loss
plt.figure(figsize=(12, 6))
plt.plot(history.history['loss'], label="Training Loss")
plt.plot(history.history['val_loss'], label="Validation Loss")
plt.xlabel("Epochs")
plt.ylabel("Loss")
plt.title("Training and Validation Loss")
plt.legend()
plt.show()
