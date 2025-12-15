# ===============================
# ETAPA 1: Construcción del .jar
# ===============================
FROM maven:3.9.9-eclipse-temurin-17 AS build

# Establece el directorio de trabajo dentro del contenedor
WORKDIR /app

# Copia los archivos de Maven y descarga dependencias
COPY pom.xml .
RUN mvn dependency:go-offline

# Copia el resto del código fuente y compila
COPY src ./src
RUN mvn clean package -Dmaven.test.skip=true


# ===============================
# ETAPA 2: Imagen final de ejecución
# ===============================
FROM eclipse-temurin:17-jdk-jammy

WORKDIR /app

# Copia el .jar construido desde la etapa anterior
COPY --from=build /app/target/app-web-jar-with-dependencies.jar app.jar

# Crea la carpeta de uploads (para archivos externos)
RUN mkdir -p /app/uploads

# Expone el puerto que usa tu servidor
EXPOSE 9001

# Variable de entorno para Render (o cualquier host)
ENV PORT=9001

# Copia el JAR principal y define el comando de inicio
ENTRYPOINT ["java", "-jar", "app.jar"]

