"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { BookOpen } from "lucide-react"
import { motion, AnimatePresence } from "framer-motion"

interface OrderLevel {
  price: number
  quantity: number
  orders: number
}

interface OrderBookData {
  bids: OrderLevel[]
  asks: OrderLevel[]
  spread: number
  best_bid: number
  best_ask: number
}

export function OrderBookMini() {
  const [orderBook, setOrderBook] = useState<OrderBookData | null>(null)
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    const fetchOrderBook = async () => {
      try {
        const response = await fetch("http://localhost:18080/orderbook?levels=5")
        if (response.ok) {
          const data = await response.json()
          setOrderBook(data.orderbook)
        }
      } catch (error) {
        console.error("Failed to fetch orderbook:", error)
      } finally {
        setLoading(false)
      }
    }

    fetchOrderBook()
    const interval = setInterval(fetchOrderBook, 2000)
    return () => clearInterval(interval)
  }, [])

  const getMaxQuantity = () => {
    if (!orderBook) return 0
    const allQuantities = [
      ...(orderBook.bids?.map((b) => b.quantity) || []),
      ...(orderBook.asks?.map((a) => a.quantity) || []),
    ]
    return Math.max(...allQuantities, 1)
  }

  return (
    <Card className="border-gray-200 h-96">
      <CardHeader>
        <CardTitle className="flex items-center gap-2 text-gray-900">
          <BookOpen className="h-5 w-5 text-blue-600" />
          Order Book
          {orderBook && (
            <Badge className="ml-2 bg-gray-50 text-gray-700 border-gray-200">
              ${orderBook.spread?.toFixed(2)} spread
            </Badge>
          )}
        </CardTitle>
      </CardHeader>
      <CardContent className="h-80">
        {loading ? (
          <div className="space-y-2">
            {[...Array(8)].map((_, i) => (
              <div key={i} className="h-8 bg-gray-100 animate-pulse rounded" />
            ))}
          </div>
        ) : !orderBook ? (
          <div className="text-center text-gray-500 py-8">No order book data</div>
        ) : (
          <div className="h-full flex flex-col">
            {/* Asks */}
            <div className="flex-1 flex flex-col-reverse">
              <AnimatePresence>
                {orderBook.asks
                  ?.slice(0, 4)
                  .reverse()
                  .map((ask, index) => (
                    <motion.div
                      key={`ask-${ask.price}`}
                      initial={{ opacity: 0, x: 20 }}
                      animate={{ opacity: 1, x: 0 }}
                      exit={{ opacity: 0, x: -20 }}
                      transition={{ delay: index * 0.05 }}
                      className="relative mb-1"
                    >
                      <div
                        className="absolute inset-0 bg-red-50 rounded"
                        style={{ width: `${(ask.quantity / getMaxQuantity()) * 100}%` }}
                      />
                      <div className="relative flex justify-between items-center p-2 text-sm hover:bg-gray-50 rounded transition-colors">
                        <span className="font-mono text-red-600">${ask.price.toFixed(2)}</span>
                        <span className="font-mono text-gray-700">{ask.quantity.toLocaleString()}</span>
                      </div>
                    </motion.div>
                  ))}
              </AnimatePresence>
            </div>

            {/* Spread */}
            <motion.div
              className="py-3 border-y border-gray-100 my-2"
              animate={{
                backgroundColor: ["rgba(59, 130, 246, 0.05)", "rgba(59, 130, 246, 0.1)", "rgba(59, 130, 246, 0.05)"],
              }}
              transition={{ duration: 2, repeat: Number.POSITIVE_INFINITY }}
            >
              <div className="text-center">
                <div className="text-xs text-gray-500">Spread</div>
                <div className="text-lg font-semibold text-blue-600">${orderBook.spread?.toFixed(2) || "0.00"}</div>
              </div>
            </motion.div>

            {/* Bids */}
            <div className="flex-1">
              <AnimatePresence>
                {orderBook.bids?.slice(0, 4).map((bid, index) => (
                  <motion.div
                    key={`bid-${bid.price}`}
                    initial={{ opacity: 0, x: -20 }}
                    animate={{ opacity: 1, x: 0 }}
                    exit={{ opacity: 0, x: 20 }}
                    transition={{ delay: index * 0.05 }}
                    className="relative mb-1"
                  >
                    <div
                      className="absolute inset-0 bg-green-50 rounded"
                      style={{ width: `${(bid.quantity / getMaxQuantity()) * 100}%` }}
                    />
                    <div className="relative flex justify-between items-center p-2 text-sm hover:bg-gray-50 rounded transition-colors">
                      <span className="font-mono text-green-600">${bid.price.toFixed(2)}</span>
                      <span className="font-mono text-gray-700">{bid.quantity.toLocaleString()}</span>
                    </div>
                  </motion.div>
                ))}
              </AnimatePresence>
            </div>
          </div>
        )}
      </CardContent>
    </Card>
  )
}
