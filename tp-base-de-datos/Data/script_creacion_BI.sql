USE GD1C2025
GO
--TABLAS DIMENSIONES

CREATE TABLE LOS_DBS.bi_tiempo (
    id_tiempo DECIMAL(18,0) not null identity(1,1),
    anio DECIMAL(18,0),
    cuatrimestre DECIMAL(18,0),
    mes DECIMAL(18,0)
);
GO

CREATE TABLE LOS_DBS.bi_Ubicacion (
    id_bi_ubicacion DECIMAL(18,0) not null identity(1,1),
    localidad_codigo DECIMAL(18,0) not null,
	localidad nvarchar(255) not null,
    provincia_codigo DECIMAL(18,0) not null,
	provincia nvarchar(255) not null
);
GO

CREATE TABLE LOS_DBS.bi_sucursal (
    id_bi_sucursal DECIMAL(18,0) not null identity(1,1),
    direccion nvarchar(255) not null,
    sucu_nro BIGINT not null
);
GO

CREATE TABLE LOS_DBS.bi_material (
    id_bi_material DECIMAL(18,0) not null identity(1,1),
    mat_codigo DECIMAL(18,0) not null,
    mat_nombre NVARCHAR(255) not null,
    mat_precio DECIMAL(38,2) not null
);
GO

CREATE TABLE LOS_DBS.bi_EstadoPedido (
    id_bi_estado_pedido DECIMAL(18,0) not null identity(1,1),
    estado_codigo DECIMAL(18,0) not null,
    estado_descripcion NVARCHAR(255) not null
);
GO

CREATE TABLE LOS_DBS.bi_turno_ventas (
    id_bi_turno_ventas DECIMAL(18,0) not null identity(1,1),
    turno nvarchar(255) not null
);
GO

CREATE TABLE LOS_DBS.bi_modelo_sillon (
	id_bi_modelo_sillon DECIMAL(18,0) not null identity(1,1),
	modelo_codigo DECIMAL(18,0) not null,
	modelo NVARCHAR(255) not null,
	descripcion NVARCHAR(255) not null,
	precio DECIMAL(18,0) not null
);
GO

CREATE TABLE LOS_DBS.bi_rango_etario_clientes (
	id_bi_rango_etario_clientes decimal(18,0) not null identity(1,1),
	descripcion varchar(255) not null
);
GO

--TABLAS HECHOS

CREATE TABLE LOS_DBS.bi_Pedido (
    id_tiempo DECIMAL (18,0) not null,
    id_bi_sucursal DECIMAL(18,0) not null,
    id_bi_turno_ventas DECIMAL(18,0) not null,
    id_bi_estado_pedido DECIMAL(18,0) not null,
	id_bi_ubicacion DECIMAL(18,0) not null,
    cantidad DECIMAL(18,0) not null
);
GO

CREATE TABLE LOS_DBS.bi_Compra (
    id_tiempo DECIMAL(18,0) not null,
    id_bi_sucursal DECIMAL(18,0) not null,
    id_bi_material DECIMAL(18,0) not null,
	id_bi_ubicacion DECIMAL(18,0) not null,
    total DECIMAL(38,2) not null,
	cantidad_compras DECIMAL(18,0) not null
);
GO

CREATE TABLE LOS_DBS.bi_Factura (
    id_tiempo DECIMAL(18,0) not null,
    id_bi_sucursal DECIMAL(18,0) not null,
	id_bi_ubicacion DECIMAL(18,0) not null,
	cantidad_facturas int not null,
    total DECIMAL(12,2) not null
);
GO


CREATE TABLE LOS_DBS.bi_Envio (
	id_tiempo DECIMAL(18,0) not null,
    id_bi_ubicacion DECIMAL(18,0) not null,
	id_bi_sucursal DECIMAL(18,0) not null,
    cantidad DECIMAL(18,0) not null,
	total_costo_envio DECIMAL(38,2) not null,
	cant_envios_cumplidos DECIMAL(18,0) not null
);
GO

CREATE TABLE LOS_DBS.bi_Venta(
	id_tiempo DECIMAL(18,0) not null,
	id_bi_modelo_sillon DECIMAL(18,0) not null,
    id_bi_sucursal DECIMAL(18,0) not null,
	id_bi_rango_Etario DECIMAL(18,0) not null,
	id_bi_ubicacion DECIMAL(18,0) not null,
	total_ventas DECIMAL(12,2) not null,
	tiempo_total_fabricacion DECIMAL(12,2) not null,
	cantidad DECIMAL(38,2) not null
);
GO
--FUNCIONES PROPIAS

CREATE FUNCTION LOS_DBS.obtener_cuatrimestre (@mes INT)
RETURNS INT
BEGIN 
	RETURN CASE
        WHEN @mes IN (1, 2, 3, 4) THEN 1
        WHEN @mes IN (5, 6, 7, 8) THEN 2
        WHEN @mes IN (9, 10, 11, 12) THEN 3
        END
END
GO

CREATE FUNCTION LOS_DBS.obtener_rango_etario (@fecha_nac DATETIME2)
RETURNS DECIMAL(18,0)
BEGIN
	DECLARE @id decimal(18,0)
	DECLARE @edad INT

	SET @edad  = YEAR(GETDATE()) - YEAR(@fecha_nac)

	IF @edad < 25
		SET @id = 1
	ELSE IF @EDAD BETWEEN  25 AND 35
		SET @id = 2
	ELSE IF @EDAD BETWEEN 35 AND 50
		SET @id = 3
	ELSE IF @EDAD > 50
		SET @id = 4

	RETURN @id
END
GO

CREATE FUNCTION LOS_DBS.obtener_turno (@fecha DATETIME2)
RETURNS decimal(18,0)
AS
BEGIN
	DECLARE @id decimal(18,0)
    DECLARE @hora INT = DATEPART(HOUR, @fecha)

    
    IF  @hora >= 8 AND @hora < 14
		SET @id = 1
	ELSE IF @hora >= 14 AND @hora < 20
		SET @id = 2

	RETURN @id
END;
GO

CREATE FUNCTION LOS_DBS.comparar_fecha(@ANIO DECIMAL(18,0),@MES DECIMAL(18,0),@CUATRI INT,@FECHA DATETIME2)
RETURNS INT
AS
BEGIN 
	RETURN CASE 
	WHEN YEAR(@FECHA) = @ANIO 
        AND LOS_DBS.obtener_cuatrimestre(MONTH(@FECHA)) = @CUATRI
        AND MONTH(@FECHA) = @MES
        THEN 1
        ELSE 0
	END
END
GO


CREATE FUNCTION LOS_DBS.comparar_turno(@TURNO decimal(18,0),@FECHA DATETIME2)
RETURNS INT
AS
BEGIN
	RETURN CASE 
	WHEN LOS_DBS.obtener_turno(@FECHA) = @TURNO 
        THEN 1
        ELSE 0
	END
END
GO

--CONSTRAINTS

ALTER TABLE LOS_DBS.bi_EstadoPedido 
ADD CONSTRAINT PK_bi_EstadoPedido PRIMARY KEY (id_bi_estado_pedido);

ALTER TABLE LOS_DBS.bi_turno_ventas 
ADD CONSTRAINT PK_bi_turno_ventas PRIMARY KEY (id_bi_turno_ventas);

ALTER TABLE LOS_DBS.bi_Ubicacion 
ADD CONSTRAINT PK_bi_ubicacion PRIMARY KEY (id_bi_ubicacion);

ALTER TABLE LOS_DBS.bi_modelo_sillon 
ADD CONSTRAINT PK_bi_modelo_sillon PRIMARY KEY (id_bi_modelo_sillon);
GO

ALTER TABLE LOS_DBS.bi_rango_etario_clientes 
ADD CONSTRAINT PK_bi_rango_etario_clientes PRIMARY KEY (id_bi_rango_etario_clientes);
GO

ALTER TABLE LOS_DBS.bi_material 
ADD CONSTRAINT PK_bi_material PRIMARY KEY (id_bi_material);
GO

ALTER TABLE LOS_DBS.bi_tiempo
ADD CONSTRAINT PK_bi_tiempo PRIMARY KEY (id_tiempo);
GO

ALTER TABLE LOS_DBS.bi_sucursal 
ADD CONSTRAINT PK_bi_sucursal PRIMARY KEY (id_bi_sucursal);


ALTER TABLE LOS_DBS.bi_Compra 
ADD CONSTRAINT PK_bi_compra PRIMARY KEY (id_tiempo, id_bi_sucursal, id_bi_material,id_bi_ubicacion);
ALTER TABLE LOS_DBS.bi_Compra 
ADD CONSTRAINT FK_bi_compra_tiempo FOREIGN KEY (id_tiempo) REFERENCES LOS_DBS.bi_tiempo(id_tiempo);
ALTER TABLE LOS_DBS.bi_Compra 
ADD CONSTRAINT FK_bi_compra_sucursal FOREIGN KEY (id_bi_sucursal) REFERENCES LOS_DBS.bi_sucursal(id_bi_sucursal);
ALTER TABLE LOS_DBS.bi_Compra 
ADD CONSTRAINT FK_bi_compra_material FOREIGN KEY (id_bi_material) REFERENCES LOS_DBS.bi_material(id_bi_material);
ALTER TABLE LOS_DBS.bi_Compra 
ADD CONSTRAINT FK_bi_compra_ubicacion FOREIGN KEY (id_bi_ubicacion) REFERENCES LOS_DBS.bi_Ubicacion(id_bi_ubicacion);

GO

ALTER TABLE LOS_DBS.bi_Factura 
ADD CONSTRAINT PK_bi_factura PRIMARY KEY (id_tiempo, id_bi_sucursal, id_bi_ubicacion);
ALTER TABLE LOS_DBS.bi_Factura 
ADD CONSTRAINT FK_bi_factura_tiempo FOREIGN KEY (id_tiempo) REFERENCES LOS_DBS.bi_tiempo(id_tiempo);
ALTER TABLE LOS_DBS.bi_Factura 
ADD CONSTRAINT FK_bi_factura_sucursal FOREIGN KEY (id_bi_sucursal) REFERENCES LOS_DBS.bi_sucursal(id_bi_sucursal);
ALTER TABLE LOS_DBS.bi_Factura 
ADD CONSTRAINT FK_bi_factura_ubicacion FOREIGN KEY (id_bi_ubicacion) REFERENCES LOS_DBS.bi_Ubicacion(id_bi_ubicacion);

GO


ALTER TABLE LOS_DBS.bi_Envio 
ADD CONSTRAINT PK_bi_envio PRIMARY KEY (id_tiempo, id_bi_ubicacion,id_bi_sucursal);
ALTER TABLE LOS_DBS.bi_Envio 
ADD CONSTRAINT FK_bi_envio_tiempo FOREIGN KEY (id_tiempo) REFERENCES LOS_DBS.bi_tiempo(id_tiempo);
ALTER TABLE LOS_DBS.bi_Envio 
ADD CONSTRAINT FK_bi_envio_ubicacion FOREIGN KEY (id_bi_ubicacion) REFERENCES LOS_DBS.bi_Ubicacion(id_bi_ubicacion);
ALTER TABLE LOS_DBS.bi_Envio 
ADD CONSTRAINT FK_bi_envio_sucursal FOREIGN KEY (id_bi_sucursal) REFERENCES LOS_DBS.bi_sucursal(id_bi_sucursal);
GO

ALTER TABLE LOS_DBS.bi_Pedido
ADD CONSTRAINT PK_bi_pedido PRIMARY KEY (id_tiempo, id_bi_sucursal, id_bi_turno_ventas, id_bi_estado_pedido,id_bi_ubicacion);
ALTER TABLE LOS_DBS.bi_Pedido
ADD CONSTRAINT FK_bi_pedido_tiempo FOREIGN KEY (id_tiempo) REFERENCES LOS_DBS.bi_tiempo(id_tiempo);
ALTER TABLE LOS_DBS.bi_Pedido
ADD CONSTRAINT FK_bi_pedido_sucursal FOREIGN KEY (id_bi_sucursal) REFERENCES LOS_DBS.bi_sucursal(id_bi_sucursal);
ALTER TABLE LOS_DBS.bi_Pedido
ADD CONSTRAINT FK_bi_pedido_turno FOREIGN KEY (id_bi_turno_ventas) REFERENCES LOS_DBS.bi_turno_ventas(id_bi_turno_ventas);
ALTER TABLE LOS_DBS.bi_Pedido
ADD CONSTRAINT FK_bi_pedido_estado FOREIGN KEY (id_bi_estado_pedido) REFERENCES LOS_DBS.bi_EstadoPedido(id_bi_estado_pedido);
ALTER TABLE LOS_DBS.bi_Pedido
ADD CONSTRAINT FK_bi_pedido_ubicacion FOREIGN KEY (id_bi_ubicacion) REFERENCES LOS_DBS.bi_Ubicacion(id_bi_ubicacion);
GO

ALTER TABLE LOS_DBS.bi_Venta 
ADD CONSTRAINT PK_bi_ventas PRIMARY KEY (id_tiempo, id_bi_modelo_sillon, id_bi_sucursal, id_bi_rango_etario,id_bi_ubicacion);
ALTER TABLE LOS_DBS.bi_Venta 
ADD CONSTRAINT FK_bi_id_tiempo FOREIGN KEY (id_tiempo) REFERENCES LOS_DBS.bi_tiempo(id_tiempo);
ALTER TABLE LOS_DBS.bi_Venta 
ADD CONSTRAINT FK_id_bi_modelo_sillon FOREIGN KEY (id_bi_modelo_sillon) REFERENCES LOS_DBS.bi_modelo_sillon(id_bi_modelo_sillon);
ALTER TABLE LOS_DBS.bi_Venta 
ADD CONSTRAINT FK_id_bi_sucursal FOREIGN KEY (id_bi_sucursal) REFERENCES LOS_DBS.bi_sucursal(id_bi_sucursal);
ALTER TABLE LOS_DBS.bi_Venta 
ADD CONSTRAINT FK_id_bi_rango_etario FOREIGN KEY (id_bi_rango_etario) REFERENCES LOS_DBS.bi_rango_etario_clientes(id_bi_rango_etario_clientes);
ALTER TABLE LOS_DBS.bi_Venta 
ADD CONSTRAINT FK_id_bi_ubicacion FOREIGN KEY (id_bi_ubicacion) REFERENCES LOS_DBS.bi_Ubicacion(id_bi_ubicacion);
GO

--PROCEDURES DIMENSIONES
CREATE PROCEDURE LOS_DBS.bi_migrar_Ubicacion
AS
BEGIN 
    INSERT INTO LOS_DBS.bi_Ubicacion(localidad_codigo,localidad,provincia_codigo,provincia)
    SELECT 
        Ubicacion.localidad_codigo,
		Ubicacion.localidad,
        Ubicacion.provincia_codigo,
		Ubicacion.provincia
    FROM (
        SELECT 
            c.Cliente_Localidad AS localidad_codigo,
			l.Localidad_Nombre as localidad,
            c.Cliente_Provincia AS provincia_codigo,
			p.Provincia_Nombre as provincia
        FROM LOS_DBS.Cliente c
		JOIN LOS_DBS.Localidad l on l.Localidad_Codigo = c.Cliente_Localidad
		JOIN LOS_DBS.Provincia p on p.Provincia_Codigo = c.Cliente_Provincia
        UNION
        SELECT
            id_localidad AS localidad_codigo,
			l.Localidad_Nombre as localidad,
            id_provincia AS provincia_codigo,
			p.Provincia_Nombre as provincia
        FROM LOS_DBS.Proveedor pr
		JOIN LOS_DBS.Localidad l on l.Localidad_Codigo = pr.id_localidad
		JOIN LOS_DBS.Provincia p on p.Provincia_Codigo = pr.id_provincia
        UNION
        SELECT
            id_localidad AS localidad_codigo,
			l.Localidad_Nombre as localidad,
            id_provincia AS provincia_codigo,
			p.Provincia_Nombre as provincia
        FROM LOS_DBS.Sucursal s
		JOIN LOS_DBS.Localidad l on l.Localidad_Codigo = s.id_localidad
		JOIN LOS_DBS.Provincia p on p.Provincia_Codigo = s.id_provincia
    ) AS Ubicacion
END;
GO



CREATE PROCEDURE LOS_DBS.bi_migrar_rango_etario
AS
BEGIN
	insert into LOS_DBS.bi_rango_etario_clientes(descripcion)
    values ('<25'),('25-35'), ('35-50'),('>50')
END
GO

CREATE PROCEDURE LOS_DBS.bi_migrar_modelo_sillon
AS
BEGIN
	INSERT INTO LOS_DBS.bi_modelo_sillon(modelo_codigo,modelo,descripcion,precio)
	SELECT  
		ms.si_modelo_codigo,
		ms.si_modelo,
		ms.si_modelo_descripcion,
		ms.si_modelo_precio
	FROM LOS_DBS.Modelo_Sillon ms
END
GO

CREATE PROCEDURE LOS_DBS.bi_migrar_material
AS
BEGIN
	INSERT INTO LOS_DBS.bi_material(mat_codigo, mat_nombre, mat_precio)
	SELECT  
		m.mat_codigo,
		m.mat_nombre,
		m.mat_precio
	FROM LOS_DBS.Material m
END
GO

CREATE PROCEDURE LOS_DBS.bi_migrar_tiempo
AS
BEGIN
    INSERT INTO LOS_DBS.bi_tiempo( anio, cuatrimestre, mes)
    SELECT DISTINCT
        anio,
        cuatrimestre,
        mes
    FROM (
        SELECT 
            YEAR(comp_fecha) AS anio,
            LOS_DBS.obtener_cuatrimestre(MONTH(comp_fecha)) AS cuatrimestre,
            MONTH(comp_fecha) AS mes
        FROM LOS_DBS.Compra
        UNION
        SELECT 
            YEAR(Factura_Fecha),
            LOS_DBS.obtener_cuatrimestre(MONTH(Factura_Fecha)),
            MONTH(Factura_Fecha)
        FROM LOS_DBS.Factura
        UNION
        SELECT 
            YEAR(Envio_Fecha),
            LOS_DBS.obtener_cuatrimestre(MONTH(Envio_Fecha)),
            MONTH(Envio_Fecha)
        FROM LOS_DBS.Envio
        UNION
        SELECT 
            YEAR(Pedido_Fecha),
            LOS_DBS.obtener_cuatrimestre(MONTH(Pedido_Fecha)),
            MONTH(Pedido_Fecha)
        FROM LOS_DBS.Pedido
    ) AS fechas_unicas
END
GO

CREATE PROCEDURE LOS_DBS.bi_migrar_turno_ventas
AS
BEGIN
    INSERT INTO LOS_DBS.bi_turno_ventas(turno)
    VALUES ('8-14'), ('14-20');
END;
GO

CREATE PROCEDURE LOS_DBS.bi_migrar_estado_pedido
AS
BEGIN
    INSERT INTO LOS_DBS.bi_EstadoPedido(estado_codigo, estado_descripcion)
    SELECT ep.Estado_Codigo, ep.Estado_Descripcion
    FROM LOS_DBS.EstadoPedido ep
END
GO


create procedure LOS_DBS.bi_migrar_Sucursal
as 
begin
	insert into LOS_DBS.bi_sucursal(direccion,sucu_nro)
	SELECT 
		s.sucu_direccion,
		s.sucu_nro
	from LOS_DBS.Sucursal s 
end;
go 


--PROCEDURES HECHOS
CREATE PROCEDURE LOS_DBS.bi_migrar_ventas
AS
BEGIN
	INSERT LOS_DBS.bi_Venta(id_tiempo,id_bi_ubicacion,id_bi_modelo_sillon,id_bi_sucursal,id_bi_rango_Etario,total_ventas,tiempo_total_fabricacion,cantidad)
	SELECT
		ti.id_tiempo,
		bi_u.id_bi_ubicacion,
		bi_ms.id_bi_modelo_sillon,
		bi_s.id_bi_sucursal,
		rangoEtario.id_bi_rango_etario_clientes,
		SUM(df.Detalle_Subtotal) as total_ventas,
		SUM(ISNULL(DATEDIFF(DAY, p.Pedido_Fecha, f.Factura_Fecha), 0)) as tiempo_total_fabricacion,
		COUNT(f.Factura_Numero) as cantidad_ventas
	FROM LOS_DBS.Factura f 
	JOIN LOS_DBS.Cliente cli on f.Factura_Cliente=cli.Cliente_Codigo
	JOIN LOS_DBS.Sucursal su ON su.sucu_nro = f.Factura_Sucursal
	JOIN LOS_DBS.Localidad l ON l.Localidad_Codigo = su.id_localidad
	JOIN LOS_DBS.DetalleFactura df ON F.Factura_Numero = df.Detalle_Factura 
	JOIN LOS_DBS.DetallePedido dp ON dp.Detalle_Codigo = df.Detalle_Pedido 
	JOIN LOS_DBS.Pedido p ON p.Pedido_Numero = dp.Detalle_Pedido
	JOIN LOS_DBS.Sillon si ON si.si_codigo = dp.Detalle_Sillon 
	JOIN LOS_DBS.Modelo_Sillon ms ON ms.si_modelo_codigo = si.si_modelo
	JOIN LOS_DBS.bi_tiempo ti ON ti.anio = YEAR(f.Factura_Fecha) and ti.cuatrimestre = LOS_DBS.obtener_cuatrimestre(MONTH(f.Factura_fecha)) and ti.mes = MONTH(f.Factura_Fecha)
	JOIN LOS_DBS.bi_modelo_sillon bi_ms ON bi_ms.modelo_codigo = si.si_modelo
	JOIN LOS_DBS.bi_sucursal bi_s ON f.Factura_Sucursal = bi_s.sucu_nro
	JOIN LOS_DBS.bi_Ubicacion bi_u ON su.id_localidad = bi_u.localidad_codigo and su.id_provincia = bi_u.provincia_codigo
	JOIN LOS_DBS.bi_rango_etario_clientes rangoEtario on rangoEtario.id_bi_rango_etario_clientes = LOS_DBS.obtener_rango_etario(cli.Cliente_FechaNacimiento)
	GROUP BY ti.id_tiempo, bi_ms.id_bi_modelo_sillon, bi_s.id_bi_sucursal, bi_u.id_bi_ubicacion ,rangoEtario.id_bi_rango_etario_clientes
END
GO



CREATE PROCEDURE LOS_DBS.bi_migrar_Factura 
AS 
BEGIN 
    INSERT INTO LOS_DBS.bi_Factura(id_tiempo, id_bi_sucursal, id_bi_ubicacion, cantidad_facturas, total)
    SELECT 
        t.id_tiempo, 
        bs.id_bi_sucursal,
        bu.id_bi_ubicacion,
        COUNT(DISTINCT f.Factura_Numero) as cantidad_facturas,
        SUM(d.Detalle_Subtotal) as total
    FROM LOS_DBS.Factura f
    JOIN LOS_DBS.Sucursal s ON s.sucu_nro = f.Factura_Sucursal
    JOIN LOS_DBS.bi_sucursal bs ON bs.sucu_nro = f.Factura_Sucursal
    JOIN LOS_DBS.bi_tiempo t ON LOS_DBS.comparar_fecha(t.anio, t.mes, t.cuatrimestre, f.Factura_Fecha) = 1
    JOIN LOS_DBS.bi_Ubicacion bu ON s.id_localidad = bu.localidad_codigo AND s.id_provincia = bu.provincia_codigo
    JOIN LOS_DBS.DetalleFactura d ON d.Detalle_Factura = f.Factura_Numero
    GROUP BY t.id_tiempo, bs.id_bi_sucursal, bu.id_bi_ubicacion
END
GO


CREATE PROCEDURE LOS_DBS.bi_migrar_envio
AS
BEGIN
    INSERT INTO LOS_DBS.bi_Envio(id_tiempo, id_bi_ubicacion, id_bi_sucursal, cantidad, total_costo_envio, cant_envios_cumplidos)
    SELECT 
        ti.id_tiempo,
        ub.id_bi_ubicacion,
        bs.id_bi_sucursal,
        COUNT(ISNULL(e.Envio_Numero,0)) AS cantidad,
        SUM(ISNULL(e.Envio_Total, 0)) as total_costo_envio,
		SUM(CASE WHEN e.Envio_Fecha IS NOT NULL 
			 AND e.Envio_Fecha_Programada IS NOT NULL 
			 AND e.Envio_Fecha = e.Envio_Fecha_Programada THEN 1 ELSE 0 END) as cant_envios_cumplidos
    FROM LOS_DBS.Envio e
    JOIN LOS_DBS.Factura f ON e.Envio_Factura = f.Factura_Numero
    JOIN LOS_DBS.bi_tiempo ti ON LOS_DBS.comparar_fecha(ti.anio, ti.mes, ti.cuatrimestre, e.Envio_Fecha) = 1
    JOIN LOS_DBS.Cliente c ON f.Factura_Cliente = c.Cliente_Codigo
    JOIN LOS_DBS.Sucursal s ON s.sucu_nro = f.Factura_Sucursal
    JOIN LOS_DBS.bi_Ubicacion ub ON c.Cliente_Localidad = ub.localidad_codigo AND c.Cliente_Provincia = ub.provincia_codigo
    JOIN LOS_DBS.bi_sucursal bs ON bs.sucu_nro = s.sucu_nro
    GROUP BY ti.id_tiempo, ub.id_bi_ubicacion, bs.id_bi_sucursal
END
GO

CREATE PROCEDURE LOS_DBS.bi_migrar_compra
AS
BEGIN
	INSERT INTO LOS_DBS.bi_Compra(id_tiempo, id_bi_sucursal, id_bi_material, id_bi_ubicacion, total, cantidad_compras)
	SELECT 
		ti.id_tiempo,
		bs.id_bi_sucursal,
		bm.id_bi_material,
		u.id_bi_ubicacion,
		SUM(ISNULL(dc.det_comp_subtotal, 0)) AS total,
		COUNT(ISNULL(c.comp_numero, 0)) AS cantidad_compras
	FROM LOS_DBS.bi_tiempo ti
	CROSS JOIN  LOS_DBS.bi_sucursal bs
	CROSS JOIN LOS_DBS.bi_material bm 
	JOIN LOS_DBS.Sucursal s ON s.sucu_nro = bs.sucu_nro
	JOIN LOS_DBS.Localidad l ON l.Localidad_Codigo = s.id_localidad
	JOIN LOS_DBS.Provincia p ON p.Provincia_Codigo = s.id_provincia
	JOIN LOS_DBS.bi_Ubicacion u ON s.id_localidad=u.localidad_codigo and p.Provincia_Codigo= u.provincia_codigo
	LEFT JOIN LOS_DBS.Compra c ON LOS_DBS.comparar_fecha(ti.anio,ti.mes,ti.cuatrimestre,c.comp_fecha) = 1 and bs.sucu_nro = c.comp_sucursal
	LEFT JOIN LOS_DBS.Detalle_compra dc ON c.comp_numero = dc.Detalle_Compra
	LEFT JOIN LOS_DBS.Material m ON dc.mat_codigo = m.mat_codigo
	GROUP BY ti.id_tiempo, bs.id_bi_sucursal, bm.id_bi_material, u.id_bi_ubicacion
	ORDER BY ti.id_tiempo, bs.id_bi_sucursal, bm.id_bi_material, u.id_bi_ubicacion
END
GO

 
CREATE PROCEDURE LOS_DBS.bi_migrar_pedido
AS
BEGIN
    INSERT INTO LOS_DBS.bi_Pedido (id_tiempo,id_bi_sucursal,id_bi_turno_ventas,id_bi_estado_pedido,id_bi_ubicacion,cantidad)
    SELECT
        ti.id_tiempo,
        bs.id_bi_sucursal,
        btv.id_bi_turno_ventas,
        bep.id_bi_estado_pedido,
		bu.id_bi_ubicacion,
        SUM(CASE WHEN P.Pedido_Numero IS NOT NULL THEN 1 ELSE 0 END) as cantidad_pedidos
    FROM LOS_DBS.bi_tiempo ti
    JOIN LOS_DBS.bi_turno_ventas btv ON 1=1
	JOIN LOS_DBS.bi_EstadoPedido bep ON 1=1
	JOIN LOS_DBS.bi_sucursal bs ON 1=1
	LEFT JOIN LOS_DBS.Sucursal s ON s.sucu_nro = bs.sucu_nro
	LEFT JOIN LOS_DBS.Localidad l ON s.id_localidad = l.Localidad_Codigo
	LEFT JOIN LOS_DBS.Provincia pr ON s.id_provincia = pr.Provincia_Codigo
	LEFT JOIN LOS_DBS.bi_Ubicacion bu ON l.Localidad_Codigo=bu.localidad_codigo and pr.Provincia_Codigo=bu.provincia_codigo
	LEFT JOIN LOS_DBS.Pedido p ON p.Pedido_Sucursal = s.sucu_nro AND
						  p.Pedido_Estado = bep.estado_codigo AND
						  LOS_DBS.comparar_turno(btv.id_bi_turno_ventas,p.Pedido_Fecha) = 1 AND 
						  LOS_DBS.comparar_fecha(ti.anio,ti.mes,ti.cuatrimestre,p.Pedido_Fecha) = 1
    GROUP BY
        ti.id_tiempo,
        bs.id_bi_sucursal,
        btv.id_bi_turno_ventas,
		bu.id_bi_ubicacion,
		bep.id_bi_estado_pedido
END
GO

EXEC LOS_DBS.bi_migrar_Ubicacion

EXEC LOS_DBS.bi_migrar_Sucursal

EXEC LOS_DBS.bi_migrar_estado_pedido

EXEC LOS_DBS.bi_migrar_modelo_sillon

EXEC LOS_DBS.bi_migrar_rango_etario

EXEC LOS_DBS.bi_migrar_tiempo

EXEC LOS_DBS.bi_migrar_material

EXEC LOS_DBS.bi_migrar_turno_ventas

EXEC LOS_DBS.bi_migrar_compra

EXEC LOS_DBS.bi_migrar_envio

EXEC LOS_DBS.bi_migrar_pedido

EXEC LOS_DBS.bi_migrar_ventas

EXEC LOS_DBS.bi_migrar_Factura

GO


--vistas 

--GANANCIAS 
CREATE VIEW LOS_DBS.vista_ganancias
AS
	SELECT
		t.mes, t.anio,s.sucu_nro,s.direccion,
		SUM(v.total_ventas) + (SELECT SUM(e.total_costo_envio) FROM LOS_DBS.bi_Envio e WHERE e.id_tiempo = v.id_tiempo AND e.id_bi_sucursal = v.id_bi_sucursal)
		- (SELECT SUM(c.total) FROM LOS_DBS.bi_Compra c WHERE c.id_tiempo = v.id_tiempo AND c.id_bi_sucursal = v.id_bi_sucursal) AS Ganancia
	FROM LOS_DBS.bi_venta v
	JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = v.id_tiempo
	JOIN LOS_DBS.bi_sucursal s ON s.id_bi_sucursal = v.id_bi_sucursal
	GROUP BY t.mes, t.anio,s.sucu_nro,s.direccion,v.id_tiempo,v.id_bi_sucursal 
GO



--FACTURA PROMEDIO MENSUAL
create view LOS_DBS.vista_promedio_mensual
as
	select 
		u.provincia_codigo,
		t.cuatrimestre,
		t.anio,
		sum(f.total) /  sum(f.cantidad_facturas) as valor_promedio_de_facturas
	from LOS_DBS.bi_Factura f
	join LOS_DBS.bi_tiempo t on f.id_tiempo = t.id_tiempo
	join LOS_DBS.bi_sucursal s on s.id_bi_sucursal = f.id_bi_sucursal
	join LOS_DBS.bi_Ubicacion u on u.id_bi_ubicacion = f.id_bi_ubicacion
	group by t.anio,t.cuatrimestre,u.provincia_codigo
go

--RENDIMIENTO DE MODELOS 
CREATE VIEW LOS_DBS.vista_rendimiento_modelos
AS
	SELECT *
	FROM (
		SELECT 
			t.cuatrimestre,
			t.anio,
			u.localidad_codigo,
			u.provincia_codigo,
			re.descripcion AS rango_etario,
			ms.modelo,
			ms.descripcion AS modelo_descripcion,
			SUM(v.cantidad) AS total_ventas,
			SUM(v.total_ventas) AS total_importe,
			ROW_NUMBER() OVER (
				PARTITION BY t.cuatrimestre, t.anio, u.localidad_codigo, re.descripcion 
				ORDER BY SUM(v.cantidad) DESC
			) AS ranking_ventas
		FROM LOS_DBS.bi_Venta v
		JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = v.id_tiempo
		JOIN LOS_DBS.bi_Ubicacion u ON u.id_bi_ubicacion = v.id_bi_ubicacion
		JOIN LOS_DBS.bi_rango_etario_clientes re ON re.id_bi_rango_etario_clientes = v.id_bi_rango_Etario
		JOIN LOS_DBS.bi_modelo_sillon ms ON ms.id_bi_modelo_sillon = v.id_bi_modelo_sillon
		GROUP BY  t.anio, t.cuatrimestre,u.localidad_codigo, u.provincia_codigo, 
				 re.descripcion, ms.modelo, ms.descripcion
	) AS sub
	WHERE sub.ranking_ventas <= 3;
GO

--VOLUMEN DE PEDIDOS 
CREATE VIEW LOS_DBS.vista_volumen_de_pedidos
AS 
	SELECT 
		tv.turno,
		s.sucu_nro,
		t.mes,
		t.anio,
		SUM(p.cantidad) as cantidad_total_pedidos
	FROM LOS_DBS.bi_Pedido p
	JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = p.id_tiempo
	JOIN LOS_DBS.bi_sucursal s ON s.id_bi_sucursal = p.id_bi_sucursal
	JOIN LOS_DBS.bi_turno_ventas tv ON tv.id_bi_turno_ventas = p.id_bi_turno_ventas
	GROUP BY tv.turno, s.sucu_nro, t.mes, t.anio;
GO

--CONVERSION DE PEDIDOS 
CREATE VIEW LOS_DBS.vista_conversion_de_pedidos
AS 
	SELECT 
		t.anio,
		t.cuatrimestre,
		s.sucu_nro,
		ep.estado_descripcion,
		SUM(p.cantidad) AS "Cantidad de pedidos en estado",
		SUM(SUM(p.cantidad)) OVER (PARTITION BY t.anio, t.cuatrimestre, s.sucu_nro) AS Cantidad_Total_Pedidos,
		CAST(SUM(p.cantidad) * 100.0 / 
			NULLIF(SUM(SUM(p.cantidad)) OVER (PARTITION BY t.cuatrimestre, t.anio, s.sucu_nro), 0)AS DECIMAL(10,2)) AS porcentaje_estado
	FROM LOS_DBS.bi_Pedido p
	JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = p.id_tiempo
	JOIN LOS_DBS.bi_sucursal s ON s.id_bi_sucursal = p.id_bi_sucursal
	JOIN LOS_DBS.bi_EstadoPedido ep ON ep.id_bi_estado_pedido = p.id_bi_estado_pedido
	GROUP BY t.anio, t.cuatrimestre,  s.sucu_nro, ep.estado_descripcion
GO

--TIEMPO PROMEDIO DE FABRICACION 
CREATE VIEW LOS_DBS.vista_tiempo_promedio_de_fabricacion
AS
	SELECT
		bt.anio,
		bt.cuatrimestre,
		bs.sucu_nro,
		SUM(bv.tiempo_total_fabricacion)/SUM(bv.cantidad) as TiempoPromedio
	FROM LOS_DBS.bi_Venta bv
	JOIN LOS_DBS.bi_sucursal bs ON bv.id_bi_sucursal = bs.id_bi_sucursal
	JOIN LOS_DBS.bi_tiempo bt ON bv.id_tiempo = bt.id_tiempo
	GROUP BY bt.anio,bt.cuatrimestre,bs.sucu_nro
GO

--PROMEDIO DE COMPRAS 
CREATE VIEW LOS_DBS.vista_importe_promedio_de_ventas
AS 
	SELECT 
		t.anio,
		t.mes,
		SUM(c.total)/SUM(c.cantidad_compras) AS importe_promedio
	FROM LOS_DBS.bi_Compra c
	JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = c.id_tiempo
	GROUP BY t.anio, t.mes;
GO


--COMPRAS POR TIPO DE MATERIAL
CREATE VIEW LOS_DBS.vista_compras_por_tipo_material
AS
	SELECT
		t.anio,
		t.cuatrimestre,
		s.sucu_nro,
		m.mat_nombre AS tipo_material,
		SUM(c.total) AS importe_total_gastado
	FROM LOS_DBS.bi_Compra c
	JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = c.id_tiempo
	JOIN LOS_DBS.bi_sucursal s ON s.id_bi_sucursal = c.id_bi_sucursal
	JOIN LOS_DBS.bi_material m ON m.id_bi_material = c.id_bi_material
	GROUP BY t.anio,t.cuatrimestre,s.sucu_nro,m.mat_nombre;
GO


--PORCENTAJE DE CUMPLIMINETO DE ENVIOS
CREATE VIEW LOS_DBS.vista_cumplimiento_envios
AS
	SELECT 
		t.anio,
		t.mes,
		SUM(e.cant_envios_cumplidos) AS envios_cumplidos,
		SUM(e.cantidad) AS envios_totales,
		CAST(SUM(e.cant_envios_cumplidos) * 100.0 / NULLIF(SUM(e.cantidad), 0) AS DECIMAL(5,2)) AS porcentaje_cumplimiento
	FROM LOS_DBS.bi_Envio e
	JOIN LOS_DBS.bi_tiempo t ON t.id_tiempo = e.id_tiempo
	JOIN LOS_DBS.bi_sucursal s ON s.id_bi_sucursal = e.id_bi_sucursal
	GROUP BY t.anio, t.mes
GO


--LOCALIDADES QUE PAGAN COSTO DE ENVIO
CREATE VIEW LOS_DBS.vista_localidades_mayor_costo_envio_combinada
AS
	SELECT TOP 3
		l.Localidad_Nombre,
		p.Provincia_Nombre,
		SUM(e.total_costo_envio)/SUM(e.cantidad) AS costo_promedio_envio
	FROM LOS_DBS.bi_Envio e
	JOIN LOS_DBS.bi_Ubicacion u ON u.id_bi_ubicacion = e.id_bi_ubicacion
	JOIN LOS_DBS.Localidad l ON u.localidad_codigo = l.Localidad_Codigo
	JOIN  LOS_DBS.Provincia p ON u.provincia_codigo = p.Provincia_Codigo
	GROUP BY l.Localidad_Nombre, p.Provincia_Nombre
	ORDER BY costo_promedio_envio DESC;
GO

