from torchvision import datasets
import torchvision.transforms as transforms
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
import struct

test_dataset = datasets.CIFAR10(root='data', train=False, transform=transforms.ToTensor(), download=True)
test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=1, shuffle=False)

labfile = open('labels.txt', 'w')

idx = 0
for (images, labels) in test_loader:
    # convert to NHWC
    images = images.permute(0, 2, 3, 1)
    # convert to a float array
    images = images.numpy()
    # pack
    packed = struct.pack(f'{images.flatten().size}f', *images.flatten())
    # write to file
    with open(f'images/{idx}.bin', 'wb') as f:
        f.write(packed)
    
    labfile.write(f'{labels[0].item()}\n')
    idx += 1


