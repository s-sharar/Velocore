"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Activity, TrendingUp } from "lucide-react"
import { motion, AnimatePresence } from "framer-motion"

interface Trade {
  trade_id: number
  buy_order_id: number
  sell_order_id: number
  symbol: string
  price: number
  quantity: number
  total_value: number
  timestamp: number
}

export function RecentTrades() {
  const [trades, setTrades] = useState<Trade[]>([])
  const [loading, setLoading] = useState(true)
  const [newTradeIds, setNewTradeIds] = useState<Set<number>>(new Set())

  useEffect(() => {
    const fetchTrades = async () => {
      try {
        const response = await fetch("http://localhost:18080/trades")
        if (response.ok) {
          const data = await response.json()
          const newTrades = (data.trades || []).slice(-10).reverse()

          // Track new trades for animation
          const currentIds = new Set(trades.map((t) => t.trade_id))
          const newIds = new Set(
            newTrades.filter((t: Trade) => !currentIds.has(t.trade_id)).map((t: Trade) => t.trade_id),
          )
          setNewTradeIds(newIds)

          setTrades(newTrades)

          // Clear new trade highlighting after animation
          setTimeout(() => setNewTradeIds(new Set()), 2000)
        }
      } catch (error) {
        console.error("Failed to fetch trades:", error)
      } finally {
        setLoading(false)
      }
    }

    fetchTrades()
    const interval = setInterval(fetchTrades, 3000)
    return () => clearInterval(interval)
  }, [trades])

  return (
    <Card className="border-gray-200 h-96">
      <CardHeader>
        <CardTitle className="flex items-center gap-2 text-gray-900">
          <Activity className="h-5 w-5 text-green-600" />
          Recent Trades
          <Badge className="ml-2 bg-green-50 text-green-700 border-green-200">{trades.length} trades</Badge>
        </CardTitle>
      </CardHeader>
      <CardContent className="h-80">
        {loading ? (
          <div className="space-y-2">
            {[...Array(8)].map((_, i) => (
              <div key={i} className="h-12 bg-gray-100 animate-pulse rounded" />
            ))}
          </div>
        ) : trades.length === 0 ? (
          <div className="text-center text-gray-500 py-8">No trades executed yet</div>
        ) : (
          <div className="space-y-2 h-full overflow-y-auto">
            <AnimatePresence>
              {trades.map((trade, index) => (
                <motion.div
                  key={trade.trade_id}
                  initial={{ opacity: 0, y: -20, scale: 0.95 }}
                  animate={{
                    opacity: 1,
                    y: 0,
                    scale: 1,
                    backgroundColor: newTradeIds.has(trade.trade_id)
                      ? ["rgba(34, 197, 94, 0.1)", "rgba(34, 197, 94, 0.05)", "rgba(255, 255, 255, 0)"]
                      : "rgba(255, 255, 255, 0)",
                  }}
                  exit={{ opacity: 0, y: 20, scale: 0.95 }}
                  transition={{
                    delay: index * 0.05,
                    backgroundColor: { duration: 2 },
                  }}
                  className="flex justify-between items-center p-3 border border-gray-100 rounded-lg hover:border-gray-200 transition-all duration-200 hover:bg-gray-50"
                >
                  <div className="flex items-center gap-3">
                    <div className="p-1.5 bg-green-50 rounded">
                      <TrendingUp className="h-3 w-3 text-green-600" />
                    </div>
                    <div>
                      <div className="flex items-center gap-2">
                        <Badge variant="outline" className="text-xs border-gray-200">
                          {trade.symbol}
                        </Badge>
                        <span className="text-xs text-gray-500">#{trade.trade_id}</span>
                      </div>
                      <div className="text-sm text-gray-700 mt-1">{trade.quantity.toLocaleString()} shares</div>
                    </div>
                  </div>

                  <div className="text-right">
                    <div className="font-mono font-semibold text-green-600">${trade.price.toFixed(2)}</div>
                    <div className="text-xs text-gray-500">${trade.total_value.toFixed(2)}</div>
                    <div className="text-xs text-gray-400 mt-1">{new Date(trade.timestamp).toLocaleTimeString()}</div>
                  </div>
                </motion.div>
              ))}
            </AnimatePresence>
          </div>
        )}
      </CardContent>
    </Card>
  )
}
