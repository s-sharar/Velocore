"use client"

import { useEffect, useState } from "react"
import { XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Area, AreaChart } from "recharts"
import { motion } from "framer-motion"

interface ChartData {
  time: string
  price: number
  timestamp: number
}

export function PriceChart() {
  const [data, setData] = useState<ChartData[]>([])
  const [currentPrice, setCurrentPrice] = useState<number>(0)

  useEffect(() => {
    const fetchMidPrice = async () => {
      try {
        const res = await fetch("http://localhost:18080/market")
        if (!res.ok) return
        const m = await res.json()
        const mid = (Number(m.best_bid || 0) + Number(m.best_ask || 0)) / 2
        const point: ChartData = {
          time: new Date().toLocaleTimeString(),
          price: Number.isFinite(mid) ? Number(mid.toFixed(4)) : 0,
          timestamp: Date.now(),
        }
        setCurrentPrice(point.price)
        setData((prev) => [...prev.slice(-119), point])
      } catch (_) {
        // ignore
      }
    }

    // Warm up with an immediate fetch and then poll
    fetchMidPrice()
    const interval = setInterval(fetchMidPrice, 2000)
    return () => clearInterval(interval)
  }, [])

  const CustomTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
      return (
        <motion.div
          initial={{ opacity: 0, scale: 0.8 }}
          animate={{ opacity: 1, scale: 1 }}
          className="bg-white border border-gray-200 rounded-lg p-3 shadow-lg"
        >
          <p className="text-gray-600 text-sm">{`Time: ${label}`}</p>
          <p className="text-blue-600 font-semibold">{`Mid Price: $${Number(payload[0].value).toFixed(4)}`}</p>
        </motion.div>
      )
    }
    return null
  }

  const gradientId = "priceGradient"

  return (
    <div className="h-full w-full">
      <ResponsiveContainer width="100%" height="100%">
        <AreaChart data={data} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
          <defs>
            <linearGradient id={gradientId} x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.3} />
              <stop offset="95%" stopColor="#3b82f6" stopOpacity={0} />
            </linearGradient>
          </defs>
          <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" opacity={0.5} />
          <XAxis dataKey="time" stroke="#6b7280" fontSize={12} tickLine={false} axisLine={false} />
          <YAxis
            stroke="#6b7280"
            fontSize={12}
            tickLine={false}
            axisLine={false}
            domain={["dataMin - 5", "dataMax + 5"]}
          />
          <Tooltip content={<CustomTooltip />} />
          <Area
            type="monotone"
            dataKey="price"
            stroke="#3b82f6"
            strokeWidth={2}
            fill={`url(#${gradientId})`}
            dot={false}
            activeDot={{
              r: 6,
              fill: "#3b82f6",
              stroke: "#ffffff",
              strokeWidth: 2,
            }}
          />
        </AreaChart>
      </ResponsiveContainer>
    </div>
  )
}
