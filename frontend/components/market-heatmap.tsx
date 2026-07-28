"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { TrendingUp, TrendingDown } from "lucide-react"
import { motion } from "framer-motion"

interface MarketItem {
  symbol: string
  price: number
  change: number
  changePercent: number
  volume: number
}

export function MarketHeatmap() {
  const [marketItems, setMarketItems] = useState<MarketItem[]>([])

  useEffect(() => {
    // Generate mock market data
    const symbols = ["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "META", "NVDA", "AMD", "NFLX"]

    const generateData = () => {
      const items = symbols.map((symbol) => {
        const basePrice = 100 + Math.random() * 200
        const change = (Math.random() - 0.5) * 10
        const changePercent = (change / basePrice) * 100

        return {
          symbol,
          price: Number(basePrice.toFixed(2)),
          change: Number(change.toFixed(2)),
          changePercent: Number(changePercent.toFixed(2)),
          volume: Math.floor(Math.random() * 1000000) + 100000,
        }
      })

      setMarketItems(items)
    }

    generateData()
    const interval = setInterval(generateData, 5000)
    return () => clearInterval(interval)
  }, [])

  return (
    <Card className="border-gray-200 h-96">
      <CardHeader>
        <CardTitle className="flex items-center gap-2 text-gray-900">
          <TrendingUp className="h-5 w-5 text-blue-600" />
          Market Overview
        </CardTitle>
      </CardHeader>
      <CardContent>
        <div className="grid grid-cols-3 gap-2 h-80 overflow-y-auto">
          {marketItems.map((item, index) => (
            <motion.div
              key={item.symbol}
              initial={{ opacity: 0, scale: 0.8 }}
              animate={{ opacity: 1, scale: 1 }}
              transition={{ delay: index * 0.1 }}
              className={`
                relative p-3 rounded-lg border transition-all duration-300 hover:scale-105 cursor-pointer
                ${
                  item.change >= 0
                    ? "bg-green-50 border-green-200 hover:border-green-300"
                    : "bg-red-50 border-red-200 hover:border-red-300"
                }
              `}
            >
              <div className="flex items-center justify-between mb-1">
                <span className="font-semibold text-gray-900 text-sm">{item.symbol}</span>
                {item.change >= 0 ? (
                  <TrendingUp className="h-3 w-3 text-green-600" />
                ) : (
                  <TrendingDown className="h-3 w-3 text-red-600" />
                )}
              </div>

              <div className="text-lg font-semibold text-gray-900 mb-1">${item.price.toFixed(2)}</div>

              <div className="flex items-center justify-between">
                <Badge
                  className={`text-xs ${
                    item.change >= 0
                      ? "bg-green-100 text-green-700 border-green-300"
                      : "bg-red-100 text-red-700 border-red-300"
                  }`}
                >
                  {item.change >= 0 ? "+" : ""}
                  {item.changePercent.toFixed(1)}%
                </Badge>
              </div>

              <div className="text-xs text-gray-500 mt-1">Vol: {(item.volume / 1000).toFixed(0)}K</div>
            </motion.div>
          ))}
        </div>
      </CardContent>
    </Card>
  )
}
