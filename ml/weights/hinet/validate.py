from torchvision import datasets
import torchvision.transforms as transforms
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np

on_gpu = torch.cuda.is_available()
device = torch.device('cuda' if on_gpu else 'cpu')

test_dataset = datasets.CIFAR10(root='data', train=False, transform=transforms.ToTensor(), download=True)
test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=1, shuffle=False)

class Net(nn.Module):
  def __init__(self):
    super(Net, self).__init__()
    self.pool1 = nn.MaxPool2d(3, 2)
    self.pool2 = nn.MaxPool2d(3, 2)
    self.pool3 = nn.MaxPool2d(3, 2)
    self.conv1 = nn.Conv2d(3,  64, 5, 1, 1)
    self.conv2 = nn.Conv2d(64, 64, 5, 1, 1)
    self.conv3 = nn.Conv2d(64, 64, 5, 1, 1)
    self.fc1 = nn.Linear(64, 10)

  def forward(self, x):
    x = self.pool1(F.relu((self.conv1(x))))
    x = self.pool2(F.relu((self.conv2(x))))
    x = self.pool3(F.relu((self.conv3(x))))
    x = x.flatten(start_dim=1)
    x = self.fc1(x)
    return x

model = Net()
model.load_state_dict(torch.load('weights.pt', map_location=device))
model.eval()

corr = 0
total = 0
for (images, labels) in test_loader:
    outputs = model(images)
    _, predicted = torch.max(outputs.data, 1)
    if predicted[0].item() == labels[0].item():
        corr += 1
    total += 1
print('Accuracy: {:.2f}%'.format(100 * corr / total))
