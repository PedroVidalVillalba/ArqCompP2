suppressPackageStartupMessages(library("viridis"))

data = read.csv("out", header = TRUE)
quantiles = aggregate(Clocks ~ Id + N + C, data, quantile)
quantiles = quantiles[order(quantiles$N, quantiles$Id, quantiles$C), ]

# Cambiar la escala de los ciclos a millones de ciclos
quantiles$Clocks = quantiles$Clocks / 1e6

### Gráfica 1: comparativa general
# Solo mostrar los datos con C = 8 hilos para el programa 4
quantiles1 = quantiles[quantiles$C == 1 | quantiles$C == 8,]

colors1 = turbo(length(unique(quantiles1$Id)))

plot(quantiles1$Clocks[,3] ~ quantiles1$N, pch = 19,
     type = 'p', col = colors1, 
     main = "Medidas de rendimiento",
     xlab = "Dimensión de las matrices (N)", 
     ylab = "Millones de ciclos de reloj")
axis(side = 1, at = unique(quantiles1$N))

for (i in 1:length(unique(quantiles1$Id))) {
    I = unique(quantiles1$Id)[i];
    x = quantiles1[quantiles1$Id == I, ]$N
    y = quantiles1[quantiles1$Id == I, ]$Clocks[,3]
    y.q1 = quantiles1[quantiles1$Id == I, ]$Clocks[,2]
    y.q3 = quantiles1[quantiles1$Id == I, ]$Clocks[,4]
    points(y ~ x, type = 'l', col = colors1[i], lwd = 2)
    arrows(x0 = x, x1 = x, y0 = y.q1, y1 = y.q3, 
           code = 3, angle = 90, length = 0.1, col = colors1[i])
}

legends = sprintf("Experimento: %s", unique(quantiles1$Id))
legend("topleft", legends, fill = colors1)

### Gráfica 2: comparativa entre distinto número de hilos variando N
quantiles20 = quantiles[quantiles$Id == "4o0",]
quantiles22 = quantiles[quantiles$Id == "4o2",]

colors20 = viridis(length(unique(quantiles20$C)))
colors22 = mako(length(unique(quantiles22$C)))

plot(quantiles20$Clocks[,3] ~ quantiles20$N, pch = 19,
     type = 'p', col = colors20, 
     main = "Comparativa de rendimiento según el número de hilos (O0)",
     xlab = "Dimensión de las matrices (N)", 
     ylab = "Millones de ciclos de reloj")
axis(side = 1, at = unique(quantiles20$N))

for (i in 1:length(unique(quantiles20$C))) {
    C = unique(quantiles20$C)[i];
    x = quantiles20[quantiles20$C == C, ]$N
    y = quantiles20[quantiles20$C == C, ]$Clocks[,3]
    y.q1 = quantiles20[quantiles20$C == C, ]$Clocks[,2]
    y.q2 = quantiles20[quantiles20$C == C, ]$Clocks[,4]
    points(y ~ x, type = 'l', col = colors20[i], lwd = 2)
    arrows(x0 = x, x1 = x, y0 = y.q1, y1 = y.q2, 
           code = 3, angle = 90, length = 0.1, col = colors20[i])
}

legends = sprintf("C = %d", unique(quantiles20$C))
legend("topleft", legends, fill = colors20)

plot(quantiles22$Clocks[,3] ~ quantiles22$N, pch = 19,
     type = 'p', col = colors22, 
     main = "Comparativa de rendimiento según el número de hilos (O2)",
     xlab = "Dimensión de las matrices (N)", 
     ylab = "Millones de ciclos de reloj")
axis(side = 1, at = unique(quantiles20$N))

for (i in 1:length(unique(quantiles22$C))) {
    C = unique(quantiles22$C)[i];
    x = quantiles22[quantiles22$C == C, ]$N
    y = quantiles22[quantiles22$C == C, ]$Clocks[,3]
    y.q1 = quantiles22[quantiles22$C == C, ]$Clocks[,2]
    y.q2 = quantiles22[quantiles22$C == C, ]$Clocks[,4]
    points(y ~ x, type = 'l', col = colors22[i], lwd = 2)
    arrows(x0 = x, x1 = x, y0 = y.q1, y1 = y.q2, 
           code = 3, angle = 90, length = 0.1, col = colors22[i])
}

legends = sprintf("C = %d", unique(quantiles22$C))
legend("topleft", legends, fill = colors22)


### Gráfica 3: ganancia en velocidad para los distintos números de hilos para N máximo
quantiles3 = quantiles[quantiles$N == max(quantiles$N) & grepl('4', quantiles$Id),]
quantiles3 = quantiles3[order(quantiles3$C, quantiles3$Id), ]

colors3 = turbo(length(unique(quantiles3$Id)))

plot(quantiles3$Clocks[,3] ~ quantiles3$C, pch = 19,
     type = 'p', col = colors3, 
     main = "Comparativa de rendimiento según el número de hilos\npara el valor más grande de N",
     xlab = "Número de hilos (C)", 
     ylab = "Millones de ciclos de reloj",
     xaxt = 'n',
     log = 'x')
axis(side = 1, at = unique(quantiles3$C))

for (i in 1:length(unique(quantiles3$Id))) {
    I = unique(quantiles3$Id)[i];
    x = quantiles3[quantiles3$Id == I, ]$C
    y = quantiles3[quantiles3$Id == I, ]$Clocks[,3]
    y.q1 = quantiles3[quantiles3$Id == I, ]$Clocks[,2]
    y.q2 = quantiles3[quantiles3$Id == I, ]$Clocks[,4]
    points(y ~ x, type = 'l', col = colors3[i], lwd = 2)
    arrows(x0 = x, x1 = x, y0 = y.q1, y1 = y.q2, 
           code = 3, angle = 90, length = 0.1, col = colors3[i])
}

legends = sprintf("Experimento: %s", unique(quantiles3$Id))
legend("topright", legends, fill = colors3)

